#pragma once

#include <cstddef>
#include <optional>
#include <string_view>

namespace source {
class Source {
  public:
    [[nodiscard]]
    virtual auto length() const -> size_t = 0;

    [[nodiscard]]
    virtual auto data() const -> const char* = 0;

    [[nodiscard]]
    virtual auto at(size_t index) const -> std::optional<char> = 0;

    [[nodiscard]]
    virtual auto range(size_t start, size_t end) const -> std::string_view = 0;

    virtual ~Source() = default;
};
}  // namespace source
