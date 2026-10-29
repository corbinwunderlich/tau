#pragma once

#include <cstddef>
#include <memory>
#include <string_view>
#include <vector>

namespace ast::pool {
class Node {
  private:
    std::unique_ptr<char[]> m_data;
    size_t m_length;

  public:
    Node(std::string_view s);

    [[nodiscard]] auto length() const -> size_t { return this->m_length; };
    [[nodiscard]] auto data() const -> char * { return this->m_data.get(); };
    [[nodiscard]] auto to_string() const -> std::string_view {
        return {this->data(), this->length()};
    };
};

class IdentPool {
  public:
    auto insert(std::string_view s) -> size_t;
    [[nodiscard]] auto get(size_t index) const -> const Node & { return this->pool[index]; };

  private:
    std::vector<Node> pool;
};

class StringPool {
  public:
    auto insert(std::string_view s) -> size_t;
    [[nodiscard]] auto get(size_t index) const -> const Node & { return this->pool[index]; };

  private:
    std::vector<Node> pool;
};
} // namespace ast::pool
