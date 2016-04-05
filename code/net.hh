/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PARASOLS_GUARD_GRAPH_NET_HH
#define PARASOLS_GUARD_GRAPH_NET_HH 1

#include "graph.hh"
#include <string>

/**
 * Read a net format file into a Graph.
 *
 * \throw GraphFileError
 */
auto read_net(const std::string & filename) -> Graph;

#endif
