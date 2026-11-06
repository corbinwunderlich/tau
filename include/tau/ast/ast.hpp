#pragma once

#include <vector>

#include "node.hpp"

namespace ast {
struct Ast {
    std::vector<node::Decl> nodes;

    [[nodiscard]]
    auto dump(const lexer::Pool& pool) const noexcept -> std::stringstream;
};
}  // namespace ast
