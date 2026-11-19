#include <ostream>

#include <tau/source/pos.hpp>
#include <tau/source/span.hpp>

namespace source {
Span::Span(Pos start, Pos end) noexcept : start_(start), end_(end) {}

Span::Span(Pos pos) noexcept : Span(pos, pos) {}

auto Span::start() const noexcept -> Pos {
    return start_;
}

auto Span::end() const noexcept -> Pos {
    return end_;
}

auto Span::operator==(const Span& other) const noexcept -> bool {
    return start_ == other.start_ && end_ == other.end_;
}

auto Span::operator!=(const Span& other) const noexcept -> bool {
    return !(*this == other);
}

auto operator<<(std::ostream& os, const Span& span) -> std::ostream& {
    return os << "Span{start: " << span.start().offset()
              << ", end: " << span.end().offset() << '}';
}
}  // namespace source
