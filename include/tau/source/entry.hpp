#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include "location.hpp"
#include "pos.hpp"

namespace source {
class Entry {
  private:
    std::string name_;
    std::string text_;
    std::vector<size_t> line_start_offsets_ = {0};

  public:
    Entry(std::string name, std::string text) noexcept;

    [[nodiscard]]
    auto name() const noexcept -> std::string_view;

    [[nodiscard]]
    auto text() const noexcept -> std::string_view;

    [[nodiscard]]
    auto at(Pos pos) const noexcept -> std::optional<char>;

    [[nodiscard]]
    auto resolve(Pos pos) const noexcept -> Location;

    [[nodiscard]]
    auto range(Pos start, size_t length) const noexcept -> std::string_view;
};
}  // namespace source
