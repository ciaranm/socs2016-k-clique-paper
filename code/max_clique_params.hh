/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PARASOLS_GUARD_MAX_CLIQUE_MAX_CLIQUE_PARAMS_HH
#define PARASOLS_GUARD_MAX_CLIQUE_MAX_CLIQUE_PARAMS_HH 1

#include "graph.hh"

#include <list>
#include <limits>
#include <chrono>
#include <atomic>
#include <vector>
#include <functional>

/**
 * Parameters for a max clique algorithm.
 *
 * Not all values make sense for all algorithms. Most of these are to allow
 * us to get more details about the search process or evaluate different
 * implementation choices, rather than key functionality.
 */
struct MaxCliqueParams
{
    /// Override the initial size of the incumbent.
    unsigned initial_bound = 0;

    /// Indicates that the graph has already been raised to this power, for
    /// s-clique (handled by the runner).
    unsigned power = 1;

    /// If this is set to true, we should abort due to a time limit.
    std::atomic<bool> * abort;

    /// The start time of the algorithm.
    std::chrono::time_point<std::chrono::steady_clock> start_time;

    /// Use lazy global domination?
    bool lgd = false;
};

#endif
