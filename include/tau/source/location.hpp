#pragma once

#include <cstddef>

namespace source {
class Line {
  private:
    size_t line_;

  public:
    friend class Location;

    explicit Line(size_t line) noexcept;
};

class Col {
  private:
    size_t col_;

  public:
    friend class Location;

    explicit Col(size_t col) noexcept;
};

class Location {
  private:
    size_t line_ = 0;
    size_t col_ = 0;

  public:
    Location() noexcept = default;
    Location(Line line, Col col) noexcept;

    [[nodiscard]]
    auto line() const noexcept -> size_t;

    [[nodiscard]]
    auto col() const noexcept -> size_t;
};
}  // namespace source
