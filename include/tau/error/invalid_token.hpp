#pragma once

#include <stdexcept>

namespace error {
class InvalidToken : public std::runtime_error {
  public:
    template <typename Found>
    InvalidToken(Found found);
};
}  // namespace error
