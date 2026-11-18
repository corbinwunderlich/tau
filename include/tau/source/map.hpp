#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "entry.hpp"

namespace source {
using Id = size_t;

class Map {
  private:
    std::vector<Entry> entries_;

  public:
    Map() noexcept = default;
    Map(std::string text) noexcept;

    auto add(Entry entry) noexcept -> Id;
    auto add(std::string name, std::string text) noexcept -> Id;

    [[nodiscard]]
    auto get(Id id) const noexcept -> const Entry&;
};
}  // namespace source
