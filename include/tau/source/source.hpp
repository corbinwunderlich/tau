#pragma once

#include <cstddef>
#include <optional>
#include <string_view>

namespace source {
class Source {
  private:
    std::string text_;

  public:
    Source(std::string text);

    [[nodiscard]]
    auto length() const noexcept -> size_t;

    [[nodiscard]]
    auto data() const noexcept -> const char*;

    [[nodiscard]]
    auto at(size_t index) const noexcept -> std::optional<char>;

    [[nodiscard]]
    auto range(size_t start, size_t end) const noexcept -> std::string_view;
};
}  // namespace source
