#pragma once

#include <stdexcept>
#include <string_view>

namespace error {
class BadEscapeSequence : public std::runtime_error {
  public:
    BadEscapeSequence();
    BadEscapeSequence(std::string_view reason);
};
}  // namespace error
