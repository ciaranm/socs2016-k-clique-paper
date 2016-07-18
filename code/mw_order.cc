/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "mw_order.hh"
#include <algorithm>

auto mw_order(const Graph & graph, std::vector<int> & p, bool reverse) -> void
{
    // pre-calculate degrees
    std::vector<int> degrees;
    std::transform(p.begin(), p.end(), std::back_inserter(degrees),
            [&] (int v) { return graph.degree(v); });

    std::vector<int> result;

    while (! p.empty()) {
        auto min = std::min_element(p.begin(), p.end(),
                [&] (int a, int b) { return degrees[a] < degrees[b] || (degrees[a] == degrees[b] && a > b); });

        result.push_back(*min);

        for (auto & v : p)
            if (graph.adjacent(*min, v))
                --degrees[v];

        p.erase(min);
    }

    if (reverse)
        std::swap(result, p);
    else
        for (auto r = result.rbegin() ; r != result.rend() ; ++r)
            p.push_back(*r);
}

