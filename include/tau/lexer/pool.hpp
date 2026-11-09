#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace lexer {
class Pool {
  private:
    std::vector<std::string_view> ident_table_;
    std::unordered_map<std::string_view, size_t> ident_map_;

    std::vector<std::string> string_table_;

  public:
    auto insert_ident(std::string_view identifier) noexcept -> size_t;

    [[nodiscard]]
    auto resolve_ident(size_t index) const noexcept -> std::string_view;

    auto insert_string(std::string string) noexcept -> size_t;

    [[nodiscard]]
    auto resolve_string(size_t index) const noexcept -> std::string;
};
}  // namespace lexer
