#include <stdexcept>
#include <string>

#include <tau/error/bad_integer_literal_prefix.hpp>

using error::BadIntegerLiteralPrefix;

namespace {
auto build_bad_integer_literal_prefix(char c) -> std::string {
    std::string message = "bad integer literal prefix `";

    message.append(1, c).append(1, '`');

    return message;
}
}  // namespace

BadIntegerLiteralPrefix::BadIntegerLiteralPrefix(char c)
    : std::runtime_error(build_bad_integer_literal_prefix(c)) {}
