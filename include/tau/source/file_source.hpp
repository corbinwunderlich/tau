#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

#include <tau/source/source.hpp>

namespace source {
class FileSource : public Source {
  private:
    std::string file;

  public:
    FileSource(std::string s);

    [[nodiscard]]
    auto length() const noexcept -> size_t override;

    [[nodiscard]]
    auto data() const noexcept -> const char* override;

    [[nodiscard]]
    auto at(size_t index) const noexcept -> std::optional<char> override;

    [[nodiscard]]
    auto range(size_t start, size_t end) const noexcept
        -> std::string_view override;
};
}  // namespace source
