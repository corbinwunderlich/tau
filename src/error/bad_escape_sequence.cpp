#include <stdexcept>
#include <string>
#include <string_view>

#include <tau/error/bad_escape_sequence.hpp>

using error::BadEscapeSequence;

namespace {
auto build_bad_escape_sequence() -> std::string {
    std::string message = "bad escape sequence";

    return message;
}

auto build_bad_escape_sequence(std::string_view reason) -> std::string {
    std::string message = "bad escape sequence: ";

    message.append(reason);

    return message;
}
}  // namespace

BadEscapeSequence::BadEscapeSequence()
    : std::runtime_error(build_bad_escape_sequence()) {}

BadEscapeSequence::BadEscapeSequence(std::string_view reason)
    : std::runtime_error(build_bad_escape_sequence(reason)) {}
