#pragma once

#include <unordered_map>
#include <vector>

namespace lexer {
class Pool {
   private:
    std::vector<std::string_view> table;
    std::unordered_map<std::string_view, size_t> map;

   public:
    auto insert(std::string_view identifier) noexcept -> size_t;
    [[nodiscard]] auto resolve(size_t index) const noexcept -> std::string_view;
};
}  // namespace lexer
