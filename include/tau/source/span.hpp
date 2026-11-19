#pragma once

#include <ostream>

#include "pos.hpp"

namespace source {
class Span {
  private:
    Pos start_;
    Pos end_;

  public:
    explicit Span(Pos start, Pos end) noexcept;
    explicit Span(Pos pos) noexcept;

    [[nodiscard]]
    auto start() const noexcept -> Pos;

    [[nodiscard]]
    auto end() const noexcept -> Pos;

    [[nodiscard]]
    auto operator==(const Span& other) const noexcept -> bool;

    [[nodiscard]]
    auto operator!=(const Span& other) const noexcept -> bool;
};

auto operator<<(std::ostream& os, const Span& span) -> std::ostream&;
}  // namespace source
