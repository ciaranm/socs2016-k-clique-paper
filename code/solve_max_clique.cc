/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "graph.hh"
#include "sequential.hh"
#include "parallel.hh"
#include "file_formats.hh"
#include "power.hh"

#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <iostream>
#include <exception>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace po = boost::program_options;

using std::chrono::steady_clock;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

namespace
{
    template <typename Result_, typename Params_, typename Data_>
    auto run_this_wrapped(const std::function<Result_ (const Data_ &, const Params_ &)> & func)
        -> std::function<Result_ (const Data_ &, Params_ &, bool &, int)>
    {
        return [func] (const Data_ & data, Params_ & params, bool & aborted, int timeout) -> Result_ {
            /* For a timeout, we use a thread and a timed CV. We also wake the
             * CV up if we're done, so the timeout thread can terminate. */
            std::thread timeout_thread;
            std::mutex timeout_mutex;
            std::condition_variable timeout_cv;
            std::atomic<bool> abort;
            abort.store(false);
            params.abort = &abort;
            if (0 != timeout) {
                timeout_thread = std::thread([&] {
                        auto abort_time = std::chrono::steady_clock::now() + std::chrono::seconds(timeout);
                        {
                            /* Sleep until either we've reached the time limit,
                             * or we've finished all the work. */
                            std::unique_lock<std::mutex> guard(timeout_mutex);
                            while (! abort.load()) {
                                if (std::cv_status::timeout == timeout_cv.wait_until(guard, abort_time)) {
                                    /* We've woken up, and it's due to a timeout. */
                                    aborted = true;
                                    break;
                                }
                            }
                        }
                        abort.store(true);
                        });
            }

            /* Start the clock */
            params.start_time = std::chrono::steady_clock::now();
            auto result = func(data, params);

            /* Clean up the timeout thread */
            if (timeout_thread.joinable()) {
                {
                    std::unique_lock<std::mutex> guard(timeout_mutex);
                    abort.store(true);
                    timeout_cv.notify_all();
                }
                timeout_thread.join();
            }

            return result;
        };
    }

    auto run_with_modifications(MaxCliqueResult func(const Graph &, const MaxCliqueParams &)) ->
        std::function<MaxCliqueResult (
                const Graph &,
                MaxCliqueParams &,
                bool &,
                int)>
    {
        return run_this_wrapped<MaxCliqueResult, MaxCliqueParams, Graph>(
                [=] (const Graph & graph, const MaxCliqueParams & params) -> MaxCliqueResult {
                    if (params.power > 1) {
                        auto power_start_time = steady_clock::now();
                        auto power_graph = power(graph, params.power);
                        auto power_time = duration_cast<milliseconds>(steady_clock::now() - power_start_time);
                        auto result = func(power_graph, params);
                        result.times.insert(result.times.begin(), power_time);
                        return result;
                    }
                    else
                        return func(graph, params);
                });
    }
}

auto main(int argc, char * argv[]) -> int
{
    try {
        po::options_description display_options{ "Program options" };
        display_options.add_options()
            ("help",                                 "Display help information")
            ("timeout",            po::value<int>(), "Abort after this many seconds")
            ("power",              po::value<int>(), "Raise the graph to this power (to solve s-clique)")
            ("format",             po::value<std::string>(), "Specify the format of the input")
            ("lgd",                                  "Use lazy global domination")
            ("parallel-search",                      "Use threaded search")
            ;

        po::options_description all_options{ "All options" };
        all_options.add_options()
            ("input-file", po::value<std::vector<std::string> >(),
                           "Specify the input file (DIMACS format, unless --format is specified). May be specified multiple times.")
            ;

        all_options.add(display_options);

        po::positional_options_description positional_options;
        positional_options
            .add("input-file", -1)
            ;

        po::variables_map options_vars;
        po::store(po::command_line_parser(argc, argv)
                .options(all_options)
                .positional(positional_options)
                .run(), options_vars);
        po::notify(options_vars);

        /* --help? Show a message, and exit. */
        if (options_vars.count("help")) {
            std::cout << "Usage: " << argv[0] << " [options] file[...]" << std::endl;
            std::cout << std::endl;
            std::cout << display_options << std::endl;
            return EXIT_SUCCESS;
        }

        /* No input file specified? Show a message and exit. */
        if (options_vars.count("input-file") < 1) {
            std::cout << "Usage: " << argv[0] << " [options] file[...]" << std::endl;
            return EXIT_FAILURE;
        }

        /* For each input file... */
        auto input_files = options_vars["input-file"].as<std::vector<std::string> >();
        bool first = true;
        for (auto & input_file : input_files) {
            if (first)
                first = false;
            else
                std::cout << "--" << std::endl;

            /* Figure out what our options should be. */
            MaxCliqueParams params;

            params.lgd = options_vars.count("lgd");

            if (options_vars.count("power"))
                params.power = options_vars["power"].as<int>();

            /* Turn a format name into a runnable function. */
            auto format = graph_file_formats.begin(), format_end = graph_file_formats.end();
            if (options_vars.count("format"))
                for ( ; format != format_end ; ++format)
                    if (format->first == options_vars["format"].as<std::string>())
                        break;

            /* Unknown format? Show a message and exit. */
            if (format == format_end) {
                std::cerr << "Unknown format " << options_vars["format"].as<std::string>() << ", choose from:";
                for (auto a : graph_file_formats)
                    std::cerr << " " << a.first;
                std::cerr << std::endl;
                return EXIT_FAILURE;
            }

            /* Read in the graph */
            auto graph = std::get<1>(*format)(input_file);

            /* Do the actual run. */
            bool aborted = false;
            auto result = run_with_modifications(options_vars.count("parallel-search") ? tcco_max_clique : cco_max_clique)(
                        graph,
                        params,
                        aborted,
                        options_vars.count("timeout") ? options_vars["timeout"].as<int>() : 0);

            /* Stop the clock. */
            auto overall_time = duration_cast<milliseconds>(steady_clock::now() - params.start_time);

            /* Display the results. */
            std::cout << result.size << " " << result.nodes;

            if (aborted)
                std::cout << " aborted";

            std::cout << std::endl;

            /* Members, and whether it's a club. */
            for (auto v : result.members)
                std::cout << graph.vertex_name(v) << " ";

            std::cout << std::endl;

            /* Times */
            std::cout << overall_time.count();
            if (! result.times.empty()) {
                for (auto t : result.times)
                    std::cout << " " << t.count();
            }
            std::cout << std::endl;
        }

        return EXIT_SUCCESS;
    }
    catch (const po::error & e) {
        std::cerr << "Error: " << e.what() << std::endl;
        std::cerr << "Try " << argv[0] << " --help" << std::endl;
        return EXIT_FAILURE;
    }
    catch (const std::exception & e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

