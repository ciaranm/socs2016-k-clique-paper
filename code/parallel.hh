/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef CODE_GUARD_PARALLEL_HH
#define CODE_GUARD_PARALLEL_HH 1

#include "graph.hh"
#include "max_clique_result.hh"
#include "max_clique_params.hh"

auto tcco_max_clique(const Graph & graph, const MaxCliqueParams & params) -> MaxCliqueResult;

#endif
