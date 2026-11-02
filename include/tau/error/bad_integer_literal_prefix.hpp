#pragma once

#include <stdexcept>

namespace error {
class BadIntegerLiteralPrefix : public std::runtime_error {
  public:
    BadIntegerLiteralPrefix(char c);
};
}  // namespace error
