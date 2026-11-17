#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <tau/source/source.hpp>

namespace source {
Source::Source(std::string text) : text_(std::move(text)) {}

auto Source::length() const noexcept -> size_t {
    return this->text_.length();
}

auto Source::data() const noexcept -> const char* {
    return this->text_.data();
}

auto Source::at(size_t index) const noexcept -> std::optional<char> {
    if (index >= this->length()) {
        return std::nullopt;
    }

    return this->text_[index];
}

auto Source::range(size_t start, size_t end) const noexcept
    -> std::string_view {
    return std::string_view(this->text_).substr(start, end);
}
}  // namespace source
