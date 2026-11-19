#include "symbols.hpp"

#include <cstddef>

namespace lexer::detail {
static_assert([]() -> bool {
    for (size_t i = 1; i < symbols.size(); i++) {
        if (symbols[i - 1].lexeme.size() < symbols[i].lexeme.size()) {
            return false;
        }
    }

    return true;
}());
}  // namespace lexer::detail
