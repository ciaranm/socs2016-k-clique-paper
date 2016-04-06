/* vim: set sw=4 sts=4 et foldmethod=syntax : */

#include "max_clique_result.hh"

auto MaxCliqueResult::merge(const MaxCliqueResult & other) -> void
{
    nodes += other.nodes;
    if (other.size > size) {
        size = other.size;
        members = std::move(other.members);
    }
}

