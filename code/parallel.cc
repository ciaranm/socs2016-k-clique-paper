/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "graph.hh"
#include "bit_graph.hh"
#include "max_clique_params.hh"
#include "max_clique_result.hh"
#include "degree_sort.hh"
#include "template_voodoo.hh"
#include "queue.hh"

#include <algorithm>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <chrono>
#include <iostream>

using std::chrono::milliseconds;
using std::chrono::steady_clock;
using std::chrono::duration_cast;

namespace
{
    const constexpr int number_of_depths = 5;
    const constexpr int number_of_steal_points = number_of_depths - 1;

    struct Subproblem
    {
        std::vector<int> offsets;
    };

    struct QueueItem
    {
        Subproblem subproblem;
    };

    struct StealPoint
    {
        std::mutex mutex;
        std::condition_variable cv;

        bool is_finished;

        bool has_data;
        std::vector<int> data;

        bool was_stolen;

        StealPoint() :
            is_finished(false),
            has_data(false),
            was_stolen(false)
        {
            mutex.lock();
        }

        void publish(std::vector<int> & s)
        {
            if (is_finished)
                return;

            data = s;
            has_data = true;
            cv.notify_all();
            mutex.unlock();
        }

        bool steal(std::vector<int> & s) __attribute__((noinline))
        {
            std::unique_lock<std::mutex> guard(mutex);

            while ((! has_data) && (! is_finished))
                cv.wait(guard);

            if (! is_finished && has_data) {
                s = data;
                was_stolen = true;
                return true;
            }
            else
                return false;
        }

        bool unpublish_and_keep_going()
        {
            if (is_finished)
                return true;

            mutex.lock();
            has_data = false;
            return ! was_stolen;
        }

        void finished()
        {
            is_finished = true;
            has_data = false;
            cv.notify_all();
            mutex.unlock();
        }
    };

    struct alignas(16) StealPoints
    {
        std::vector<StealPoint> points;

        StealPoints() :
            points{ number_of_steal_points }
        {
        }
    };

    struct AtomicIncumbent
    {
        std::atomic<unsigned> value;

        AtomicIncumbent()
        {
            value.store(0, std::memory_order_seq_cst);
        }

        bool update(unsigned v)
        {
            while (true) {
                unsigned cur_v = value.load(std::memory_order_seq_cst);
                if (v > cur_v) {
                    if (value.compare_exchange_strong(cur_v, v, std::memory_order_seq_cst))
                        return true;
                }
                else
                    return false;
            }
        }

        bool beaten_by(unsigned v) const
        {
            return v > value.load(std::memory_order_seq_cst);
        }

        unsigned get() const
        {
            return value.load(std::memory_order_relaxed);
        }
    };

    template <unsigned size_>
    struct TCCO
    {
        const Graph & original_graph;
        FixedBitGraph<size_> graph;
        const MaxCliqueParams & params;
        std::vector<int> order;

        MaxCliqueResult result;

        std::list<std::set<int> > previouses;

        std::vector<std::pair<std::atomic<bool>, FixedBitSet<size_> > > unsets;
        std::mutex unsets_mutex;

        AtomicIncumbent best_anywhere; // global incumbent

        TCCO(const Graph & g, const MaxCliqueParams & p) :
            original_graph(g),
            params(p),
            order(g.size()),
            unsets(g.size())
        {
            // populate our order with every vertex initially
            std::iota(order.begin(), order.end(), 0);
            degree_sort(g, order, false);

            // re-encode graph as a bit graph
            graph.resize(g.size());

            for (int i = 0 ; i < g.size() ; ++i)
                for (int j = 0 ; j < g.size() ; ++j)
                    if (g.adjacent(order[i], order[j]))
                        graph.add_edge(i, j);
        }

        template <typename... MoreArgs_>
        auto expand(
                std::vector<unsigned> & c,
                FixedBitSet<size_> & p,
                const std::array<BitWord, size_ * bits_per_word> & p_order,
                const std::array<BitWord, size_ * bits_per_word> & colours,
                std::vector<int> & position,
                MoreArgs_ && ... more_args_
                ) -> void
        {
            increment_nodes(std::forward<MoreArgs_>(more_args_)...);

            int skip = 0, stop = std::numeric_limits<int>::max();
            bool keep_going = true;
            get_skip_and_stop(c.size(), std::forward<MoreArgs_>(more_args_)..., skip, stop, keep_going);

            int previous_v = -1;

            // for each v in p... (v comes later)
            for (int n = p.popcount() - 1 ; n >= 0 ; --n) {
                // bound, timeout or early exit?
                unsigned best_anywhere_value = get_best_anywhere_value();
                if (c.size() + colours[n] <= best_anywhere_value || params.abort->load())
                    return;

                if (params.lgd && -1 != previous_v)
                    propagate_lazy_global_domination(previous_v, p);

                auto v = p_order[n];
                previous_v = v;

                if (skip > 0 || should_skip(v, p)) {
                    --skip;
                    p.unset(v);
                }
                else {
                    // consider taking v
                    c.push_back(v);

                    // filter p to contain vertices adjacent to v
                    FixedBitSet<size_> new_p = p;
                    graph.intersect_with_row(v, new_p);

                    if (new_p.empty()) {
                        potential_new_best(c, position, std::forward<MoreArgs_>(more_args_)...);
                    }
                    else {
                        position.push_back(0);
                        std::array<BitWord, size_ * bits_per_word> new_p_order;
                        std::array<BitWord, size_ * bits_per_word> new_colours;
                        colour_class_order(new_p, new_p_order, new_colours);
                        keep_going = recurse(
                                c, new_p, new_p_order, new_colours, position, std::forward<MoreArgs_>(more_args_)...) && keep_going;
                        position.pop_back();
                    }

                    // now consider not taking v
                    c.pop_back();
                    p.unset(v);

                    keep_going = keep_going && (--stop > 0);

                    if (! keep_going)
                        break;
                }
            }
        }

        auto get_best_anywhere_value() -> unsigned
        {
            return best_anywhere.get();
        }

        auto recurse(
                std::vector<unsigned> & c,
                FixedBitSet<size_> & p,
                const std::array<BitWord, size_ * bits_per_word> & initial_p_order,
                const std::array<BitWord, size_ * bits_per_word> & initial_colours,
                std::vector<int> & position,
                MaxCliqueResult & local_result,
                Subproblem * const subproblem,
                StealPoints * const steal_points
                ) -> bool
        {
            if (steal_points && c.size() < number_of_steal_points)
                steal_points->points.at(c.size() - 1).publish(position);

            expand(c, p, initial_p_order, initial_colours, position, local_result,
                subproblem && c.size() < subproblem->offsets.size() ? subproblem : nullptr,
                steal_points && c.size() < number_of_steal_points ? steal_points : nullptr);

            if (steal_points && c.size() < number_of_steal_points)
                return steal_points->points.at(c.size() - 1).unpublish_and_keep_going();
            else
                return true;
        }

        auto increment_nodes(
                MaxCliqueResult & local_result,
                Subproblem * const,
                StealPoints * const
                ) -> void
        {
            ++local_result.nodes;
        }

        auto run() -> MaxCliqueResult
        {
            MaxCliqueResult global_result;
            global_result.size = params.initial_bound;
            std::mutex global_result_mutex;

            /* work queues */
            std::vector<std::unique_ptr<Queue<QueueItem> > > queues;
            for (unsigned depth = 0 ; depth < number_of_depths ; ++depth)
                queues.push_back(std::unique_ptr<Queue<QueueItem> >{ new Queue<QueueItem>{ std::thread::hardware_concurrency(), false } });

            /* initial job */
            queues[0]->enqueue(QueueItem{ Subproblem{ std::vector<int>{} } });
            if (queues[0]->want_producer())
                queues[0]->initial_producer_done();

            /* threads and steal points */
            std::list<std::thread> threads;
            std::vector<StealPoints> thread_steal_points(std::thread::hardware_concurrency());

            // initial colouring
            std::array<BitWord, size_ * bits_per_word> initial_p_order;
            std::array<BitWord, size_ * bits_per_word> initial_colours;
            {
                FixedBitSet<size_> initial_p;
                initial_p.set_up_to(graph.size());
                colour_class_order(initial_p, initial_p_order, initial_colours);
            }

            // snoopy
            std::mutex snoop_mutex;
            std::condition_variable snoop_cv;
            bool snoops_changed = false, finished = false;
            std::vector<Subproblem> snoops(std::thread::hardware_concurrency());

            /* workers */
            for (unsigned i = 0 ; i < std::thread::hardware_concurrency() ; ++i) {
                threads.push_back(std::thread([&, i] {
                            auto start_time = steady_clock::now(); // local start time
                            auto overall_time = duration_cast<milliseconds>(steady_clock::now() - start_time);

                            MaxCliqueResult local_result; // local result

                            for (unsigned depth = 0 ; depth < number_of_depths ; ++depth) {
                                if (queues[depth]->want_producer()) {
                                    /* steal */
                                    for (unsigned j = 0 ; j < std::thread::hardware_concurrency() ; ++j) {
                                        if (j == i)
                                            continue;

                                        std::vector<int> stole;
                                        if (thread_steal_points.at(j).points.at(depth - 1).steal(stole)) {
                                            stole.pop_back();
                                            for (auto & s : stole)
                                                --s;
                                            while (++stole.back() < graph.size())
                                                queues[depth]->enqueue(QueueItem{ Subproblem{ stole } });
                                        }
                                    }
                                    queues[depth]->initial_producer_done();
                                }

                                while (true) {
                                    // get some work to do
                                    QueueItem args;
                                    if (! queues[depth]->dequeue_blocking(args))
                                        break;

                                    std::vector<unsigned> c;
                                    c.reserve(graph.size());

                                    FixedBitSet<size_> p; // local potential additions
                                    p.set_up_to(graph.size());

                                    std::vector<int> position;
                                    position.reserve(graph.size());
                                    position.push_back(0);

                                    {
                                        std::unique_lock<std::mutex> guard(snoop_mutex);
                                        snoops.at(i) = args.subproblem;
                                        snoops_changed = true;
                                    }

                                    // do some work
                                    expand(c, p, initial_p_order, initial_colours, position, local_result,
                                            &args.subproblem, &thread_steal_points.at(i));

                                    // record the last time we finished doing useful stuff
                                    overall_time = duration_cast<milliseconds>(steady_clock::now() - start_time);
                                }

                                if (depth < number_of_steal_points)
                                    thread_steal_points.at(i).points.at(depth).finished();
                            }

                            // merge results
                            {
                                std::unique_lock<std::mutex> guard(global_result_mutex);
                                global_result.merge(local_result);
                                global_result.times.push_back(overall_time);
                            }
                            }));
            }

            // snoop thread
            std::thread snoop_thread([&] {
                    while (! finished) {
                        auto abort_time = std::chrono::steady_clock::now() + std::chrono::seconds(1);
                        std::unique_lock<std::mutex> guard(snoop_mutex);
                        if (std::cv_status::timeout == snoop_cv.wait_until(guard, abort_time) && snoops_changed) {
                            snoops_changed = false;
                            std::cerr << "active:";
                            for (auto & s : snoops) {
                                std::cerr << " ( ";
                                for (auto & p : s.offsets)
                                    std::cerr << p << " ";
                                std::cerr << ")";
                            }
                            std::cerr << " bound " << get_best_anywhere_value();
                            std::cerr << std::endl;
                        }
                    }
                    });

            // wait until they're done, and clean up threads
            for (auto & t : threads)
                t.join();

            // kill the snooper
            {
                std::unique_lock<std::mutex> guard(snoop_mutex);
                finished = true;
                snoop_cv.notify_all();
            }
            snoop_thread.join();

            return global_result;
        }

        auto potential_new_best(
                const std::vector<unsigned> & c,
                const std::vector<int> &,
                MaxCliqueResult & local_result,
                Subproblem * const,
                StealPoints * const
                ) -> void
        {
            if (best_anywhere.update(c.size())) {
                local_result.size = c.size();
                local_result.members.clear();
                for (auto & v : c)
                    local_result.members.insert(order[v]);
            }
        }

        auto potential_new_best(const std::vector<unsigned> & c) -> void
        {
            if (c.size() > result.size) {
                result.size = c.size();

                result.members.clear();
                for (auto & v : c)
                    result.members.insert(order[v]);
            }
        }

        void propagate_lazy_global_domination(BitWord v, FixedBitSet<size_> & p)
        {
            if (! unsets[v].first) {
                std::unique_lock<std::mutex> guard(unsets_mutex);

                if (! unsets[v].first) {
                    FixedBitSet<size_> nv = graph.neighbourhood(v);

                    for (unsigned i = 0 ; i < unsigned(graph.size()) ; ++i) {
                        if (i == v)
                            continue;

                        FixedBitSet<size_> niv = graph.neighbourhood(i);
                        niv.intersect_with_complement(nv);
                        niv.unset(v);
                        if (niv.empty())
                            unsets[v].second.set(i);
                    }

                    unsets[v].first = true;
                }
            }

            p.intersect_with_complement(unsets[v].second);
        }

        auto should_skip(BitWord v, FixedBitSet<size_> & p) -> bool
        {
            return ! p.test(v);
        }

        auto get_skip_and_stop(
                unsigned c_popcount,
                MaxCliqueResult &,
                Subproblem * const subproblem,
                StealPoints * const,
                int & skip,
                int &,
                bool & keep_going
                ) -> void
        {
            if (subproblem && c_popcount < subproblem->offsets.size()) {
                skip = subproblem->offsets.at(c_popcount);
                keep_going = false;
            }
        }

        auto colour_class_order(
                const FixedBitSet<size_> & p,
                std::array<BitWord, size_ * bits_per_word> & p_order,
                std::array<BitWord, size_ * bits_per_word> & p_bounds) -> void
        {
            FixedBitSet<size_> p_left = p; // not coloured yet
            BitWord colour = 0;        // current colour
            BitWord i = 0;             // position in p_bounds

            // while we've things left to colour
            while (! p_left.empty()) {
                // next colour
                ++colour;
                // things that can still be given this colour
                FixedBitSet<size_> q = p_left;

                // while we can still give something this colour
                while (! q.empty()) {
                    // first thing we can colour
                    int v = q.first_set_bit();
                    p_left.unset(v);
                    q.unset(v);

                    // can't give anything adjacent to this the same colour
                    graph.intersect_with_row_complement(v, q);

                    // record in result
                    p_bounds[i] = colour;
                    p_order[i] = v;
                    ++i;
                }
            }
        }
    };
}

auto tcco_max_clique(const Graph & graph, const MaxCliqueParams & params) -> MaxCliqueResult
{
    return select_graph_size<TCCO, MaxCliqueResult>(AllGraphSizes(), graph, params);
}

