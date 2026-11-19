#pragma once

#include <cstddef>

namespace source {
class Pos {
  private:
    size_t offset_ = 0;

  public:
    Pos() noexcept = default;
    Pos(size_t offset) noexcept;

    [[nodiscard]]
    auto offset() const noexcept -> size_t;

    auto advance(size_t by = 1) noexcept -> void;

    [[nodiscard]]
    auto operator+(const Pos& rhs) const noexcept -> Pos;

    [[nodiscard]]
    auto operator-(const Pos& rhs) const noexcept -> Pos;

    [[nodiscard]]
    auto operator==(const Pos& other) const noexcept -> bool;

    [[nodiscard]]
    auto operator!=(const Pos& other) const noexcept -> bool;
};
}  // namespace source
