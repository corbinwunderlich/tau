#pragma once

#include <cstddef>
#include <memory>

#include "node.hpp"

namespace ast {
struct Ast {
    std::unique_ptr<node::Decl[]> nodes;
    size_t node_count;
};
} // namespace ast
