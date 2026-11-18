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
}  // namespace source
