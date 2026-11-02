#pragma once

#include <stdexcept>

#include <tau/lexer/base.hpp>

namespace error {
class BadIntegerLiteral : public std::runtime_error {
  public:
    BadIntegerLiteral(lexer::Base base);
};
}  // namespace error
