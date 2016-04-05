/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PARASOLS_GUARD_GRAPH_DEGREE_SORT_HH
#define PARASOLS_GUARD_GRAPH_DEGREE_SORT_HH 1

#include "graph.hh"

#include <vector>

/**
 * Sort the vertices of p in non-decreasing degree order (or non-increasing
 * if 'reverse' is true), tie-breaking on vertex number.
 */
auto degree_sort(const Graph & graph, std::vector<int> & p, bool reverse) -> void;

#endif
