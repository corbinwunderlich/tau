#include <ostream>
#include <string_view>
#include <type_traits>

#include <tau/lexer/token.hpp>

namespace lexer {
auto operator<<(std::ostream& os, const Token& token) -> std::ostream& {
    return os << token.visit([](auto&& arg) -> std::string_view {
        const token::Token<std::decay_t<decltype(arg)>>& token = arg;

        return token.description();
    });
}
}  // namespace lexer
