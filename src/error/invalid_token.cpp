#include <stdexcept>
#include <string>

#include <tau/error/invalid_token.hpp>

using error::InvalidToken;

namespace {
[[maybe_unused]]
auto quote_invalid_token(std::string_view sv) -> std::string {
    return std::string().append(sv);
}

[[maybe_unused]]
auto quote_invalid_token(char c) -> std::string {
    return std::string{"`"} + c + '`';
}

template <typename Found>
auto build_invalid_token(Found found) -> std::string {
    std::string message =
        "expected valid token, found " + quote_invalid_token(found);

    return message;
}
}  // namespace

template <typename Found>
InvalidToken::InvalidToken(Found found)
    : std::runtime_error(build_invalid_token(found)) {}
