/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "graph.hh"
#include "bit_graph.hh"
#include "max_clique_params.hh"
#include "max_clique_result.hh"
#include "degree_sort.hh"
#include "template_voodoo.hh"

#include <algorithm>

namespace
{
    template <unsigned size_>
    struct CCO
    {
        const Graph & original_graph;
        FixedBitGraph<size_> graph;
        const MaxCliqueParams & params;
        std::vector<int> order;

        MaxCliqueResult result;

        std::list<std::set<int> > previouses;

        std::vector<std::pair<bool, FixedBitSet<size_> > > unsets;

        CCO(const Graph & g, const MaxCliqueParams & p) :
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

        auto expand(
                std::vector<unsigned> & c,
                FixedBitSet<size_> & p,
                const std::array<BitWord, size_ * bits_per_word> & p_order,
                const std::array<BitWord, size_ * bits_per_word> & colours
                ) -> void
        {
            ++result.nodes;

            int previous_v = -1;

            // for each v in p... (v comes later)
            for (int n = p.popcount() - 1 ; n >= 0 ; --n) {
                // bound, timeout or early exit?
                unsigned best_anywhere_value = result.size;
                if (c.size() + colours[n] <= best_anywhere_value || params.abort->load())
                    return;

                if (params.lgd && -1 != previous_v)
                    propagate_lazy_global_domination(previous_v, p);

                auto v = p_order[n];
                previous_v = v;

                if (skip(v, p)) {
                    p.unset(v);
                }
                else {
                    // consider taking v
                    c.push_back(v);

                    // filter p to contain vertices adjacent to v
                    FixedBitSet<size_> new_p = p;
                    graph.intersect_with_row(v, new_p);

                    if (new_p.empty()) {
                        potential_new_best(c);
                    }
                    else {
                        std::array<BitWord, size_ * bits_per_word> new_p_order;
                        std::array<BitWord, size_ * bits_per_word> new_colours;
                        colour_class_order(new_p, new_p_order, new_colours);
                        expand(c, new_p, new_p_order, new_colours);
                    }

                    // now consider not taking v
                    c.pop_back();
                    p.unset(v);
                }
            }
        }

        auto run() -> MaxCliqueResult
        {
            result.size = params.initial_bound;

            std::vector<unsigned> c;
            c.reserve(graph.size());

            FixedBitSet<size_> p; // potential additions
            p.set_up_to(graph.size());

            // initial colouring
            std::array<BitWord, size_ * bits_per_word> initial_p_order;
            std::array<BitWord, size_ * bits_per_word> initial_colours;
            colour_class_order(p, initial_p_order, initial_colours);

            // go!
            expand(c, p, initial_p_order, initial_colours);

            return result;
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
                unsets[v].first = true;

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
            }

            p.intersect_with_complement(unsets[v].second);
        }

        auto skip(BitWord v, FixedBitSet<size_> & p) -> bool
        {
            return ! p.test(v);
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

auto cco_max_clique(const Graph & graph, const MaxCliqueParams & params) -> MaxCliqueResult
{
    return select_graph_size<CCO, MaxCliqueResult>(AllGraphSizes(), graph, params);
}

