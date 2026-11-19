#include <cstddef>

#include <tau/source/location.hpp>

namespace source {
Line::Line(size_t line) noexcept : line_(line) {}
Col::Col(size_t col) noexcept : col_(col) {}

Location::Location(Line line, Col col) noexcept
    : line_(line.line_), col_(col.col_) {}

auto Location::line() const noexcept -> size_t {
    return line_;
}

auto Location::col() const noexcept -> size_t {
    return col_;
}
}  // namespace source
