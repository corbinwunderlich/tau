#pragma once

#include <vector>

#include "node.hpp"

namespace ast {
struct Ast {
    std::vector<node::Decl> nodes;
};
}  // namespace ast
