#include <stdexcept>
#include <string>

#include <tau/error/bad_integer_literal.hpp>
#include <tau/lexer/base.hpp>

using error::BadIntegerLiteral;
using lexer::Base;

namespace {
auto build_bad_integer_literal(Base base) -> std::string {
    std::string message = "bad integer literal: invalid ";

    switch (base) {
        case Base::Binary:
            message.append("binary");
            break;
        case Base::Octal:
            message.append("octal");
            break;
        case Base::Decimal:
            message.append("decimal");
            break;
        case Base::Hexadecimal:
            message.append("hex");
            break;
    }

    message.append(" code");

    return message;
}
}  // namespace

BadIntegerLiteral::BadIntegerLiteral(Base base)
    : std::runtime_error(build_bad_integer_literal(base)) {};
