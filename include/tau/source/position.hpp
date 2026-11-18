#pragma once

#include <cstddef>

namespace source {
class Position {
  private:
    size_t line_ = 0;
    size_t col_ = 0;

  public:
    Position() noexcept = default;
    Position(size_t line, size_t col) noexcept;

    [[nodiscard]]
    auto line() const noexcept -> size_t;

    [[nodiscard]]
    auto col() const noexcept -> size_t;
};
}  // namespace source
