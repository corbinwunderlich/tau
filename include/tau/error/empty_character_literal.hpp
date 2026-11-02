#pragma once

#include <stdexcept>

namespace error {
class EmptyCharacterLiteral : public std::runtime_error {
  public:
    EmptyCharacterLiteral();
};
}  // namespace error
