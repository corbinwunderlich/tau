#include <cstddef>

#include <tau/source/pos.hpp>

namespace source {
Pos::Pos(size_t offset) noexcept : offset_(offset) {}

auto Pos::offset() const noexcept -> size_t {
    return offset_;
}

auto Pos::advance(size_t by) noexcept -> void {
    offset_ += by;
}

auto Pos::operator+(const Pos& rhs) const noexcept -> Pos {
    return {offset_ + rhs.offset_};
}

auto Pos::operator-(const Pos& rhs) const noexcept -> Pos {
    return {offset_ - rhs.offset_};
}

auto Pos::operator==(const Pos& other) const noexcept -> bool {
    return this->offset_ == other.offset_;
}

auto Pos::operator!=(const Pos& other) const noexcept -> bool {
    return !(*this == other);
}
}  // namespace source
