#include <stdexcept>
#include <string>

#include <tau/error/empty_character_literal.hpp>

using error::EmptyCharacterLiteral;

namespace {
auto build_empty_character_literal() -> std::string {
    std::string message = "empty character literals are not valid";

    return message;
}
}  // namespace

EmptyCharacterLiteral::EmptyCharacterLiteral()
    : std::runtime_error(build_empty_character_literal()) {}
