#include <algorithm>
#include <cstddef>
#include <iterator>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <tau/source/entry.hpp>
#include <tau/source/pos.hpp>
#include <tau/source/position.hpp>

namespace source {
Entry::Entry(std::string name, std::string text) noexcept
    : name_(std::move(name)), text_(std::move(text)) {
    size_t pos = text_.find('\n', 0);

    while (pos != std::string::npos && pos + 1 < text.length()) {
        line_start_offsets_.push_back(pos);

        pos = text_.find('\n', pos + 1);
    }

    line_start_offsets_.shrink_to_fit();
}

auto Entry::name() const noexcept -> std::string_view {
    return name_;
}

auto Entry::text() const noexcept -> std::string_view {
    return text_;
}

auto Entry::at(Pos pos) const noexcept -> std::optional<char> {
    if (pos.offset() >= text_.length()) {
        return std::nullopt;
    }

    return text_[pos.offset()];
}

auto Entry::resolve(Pos pos) const noexcept -> Position {
    const size_t offset = std::min(pos.offset(), text_.size());

    const auto next_line_offset = std::upper_bound(
        line_start_offsets_.begin(), line_start_offsets_.end(), offset);

    const auto line = static_cast<size_t>(
        std::distance(line_start_offsets_.begin(), next_line_offset) - 1);

    return Position{line, offset - line_start_offsets_[line]};
}

auto Entry::range(Pos start, size_t length) const noexcept -> std::string_view {
    return std::string_view(text_).substr(start.offset(), length);
}
}  // namespace source
