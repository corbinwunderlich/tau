#include <algorithm>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <memory>
#include <string_view>

#include <tau/ast/pool.hpp>

namespace pool = ast::pool;

pool::Node::Node(std::string_view s) {
    this->m_data = std::make_unique<char[]>(s.size());

    if (!s.empty()) {
        std::memcpy(this->m_data.get(), s.data(), s.size());
    }

    this->m_length = s.length();
}

auto pool::IdentPool::insert(std::string_view s) -> size_t {
    auto existing_node = std::find_if(this->pool.begin(), this->pool.end(),
                                      [&s](const Node &node) -> bool {
                                          if (s.length() != node.length()) {
                                              return false;
                                          }

                                          return std::memcmp(s.data(), node.data(), s.size()) == 0;
                                      });

    if (existing_node != this->pool.end()) {
        return static_cast<size_t>(std::distance(this->pool.begin(), existing_node));
    }

    this->pool.emplace_back(s);

    return this->pool.size() - 1;
}

auto pool::StringPool::insert(std::string_view s) -> size_t {
    this->pool.emplace_back(s);

    return this->pool.size() - 1;
}
