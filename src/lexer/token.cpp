#include <ostream>
#include <string_view>

#include <tau/lexer/token.hpp>

namespace lexer {
auto operator<<(std::ostream& os, const Token& token) -> std::ostream& {
    return os << token.visit([](auto&& arg) -> std::string_view {
        return arg.description();
    });
}
}  // namespace lexer
