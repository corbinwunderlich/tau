#pragma once

#include <optional>
#include <string_view>

#include <tau/lexer/token.hpp>
#include <tau/source/span.hpp>

namespace lexer::keyword {
[[nodiscard]]
auto lookup(source::Span span, std::string_view identifier)
    -> std::optional<Token>;
}  // namespace lexer::keyword
