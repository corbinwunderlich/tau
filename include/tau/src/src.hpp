#pragma once

#include <optional>
namespace src {
class Sources {
   private:
    const std::string source;

   public:
    Sources(std::string source_file) : source(std::move(source_file)) {};
    [[nodiscard]] auto length() const noexcept -> size_t {
        return this->source.length();
    }
    [[nodiscard]] auto data() const noexcept -> const char* {
        return this->source.data();
    }
    [[nodiscard]] auto at(size_t index) const noexcept -> std::optional<char> {
        if (index >= this->length()) {
            return std::nullopt;
        }

        return this->source[index];
    }
    [[nodiscard]] auto range(size_t start, size_t end) const noexcept
        -> std::string_view {
        return std::string_view(this->source).substr(start, end);
    }
};
}  // namespace src
