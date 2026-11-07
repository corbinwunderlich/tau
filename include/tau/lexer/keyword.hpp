#pragma once

#include <optional>
#include <string_view>

#include <tau/lexer/token.hpp>

namespace lexer::keyword {
[[nodiscard]]
auto lookup(std::string_view identifier) -> std::optional<Token>;
}  // namespace lexer::keyword
