/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PARASOLS_GUARD_GRAPH_MW_ORDER_HH
#define PARASOLS_GUARD_GRAPH_MW_ORDER_HH 1

#include "graph.hh"

#include <vector>

auto mw_order(const Graph & graph, std::vector<int> & p, bool reverse) -> void;

#endif
