#include <cstddef>

#include <tau/source/position.hpp>

namespace source {
Position::Position(size_t line, size_t col) noexcept : line_(line), col_(col) {}

auto Position::line() const noexcept -> size_t {
    return line_;
}

auto Position::col() const noexcept -> size_t {
    return col_;
}
}  // namespace source
