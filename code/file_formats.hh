/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#ifndef PARASOLS_GUARD_GRAPH_FILE_FORMATS_HH
#define PARASOLS_GUARD_GRAPH_FILE_FORMATS_HH 1

#include "graph.hh"
#include "net.hh"
#include "dimacs.hh"
#include "metis.hh"

#include <utility>
#include <functional>

namespace detail
{
    using GraphFileFormatFunction = std::function<Graph (const std::string &)>;

    using namespace std::placeholders;

    auto graph_file_formats = {
        std::make_pair( std::string{ "dimacs" },  GraphFileFormatFunction{ std::bind(read_dimacs, _1) } ),
        std::make_pair( std::string{ "net" },     GraphFileFormatFunction{ std::bind(read_net, _1) } ),
        std::make_pair( std::string{ "metis" },   GraphFileFormatFunction{ std::bind(read_metis, _1) } )
    };
}

using detail::graph_file_formats;

#endif
