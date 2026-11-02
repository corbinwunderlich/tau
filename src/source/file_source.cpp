#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include <tau/source/file_source.hpp>

namespace source {
FileSource::FileSource(std::string s) : file(std::move(s)) {}

auto FileSource::length() const noexcept -> size_t {
    return this->file.length();
}

auto FileSource::data() const noexcept -> const char* {
    return this->file.data();
}

auto FileSource::at(size_t index) const noexcept -> std::optional<char> {
    if (index >= this->length()) {
        return std::nullopt;
    }

    return this->file[index];
}

auto FileSource::range(size_t start, size_t end) const noexcept
    -> std::string_view {
    return std::string_view(this->file).substr(start, end);
}
}  // namespace source
