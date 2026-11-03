#include <string_view>

#include <tau/lexer/pool.hpp>

using lexer::Pool;

auto Pool::insert(std::string_view identifier) noexcept -> size_t {
    const auto existing = this->map_.find(identifier);

    if (existing != this->map_.end()) {
        return existing->second;
    }

    const auto index = this->table_.size();

    this->table_.emplace_back(identifier);
    this->map_.insert({identifier, index});

    return index;
}

auto Pool::resolve(size_t index) const noexcept -> std::string_view {
    return this->table_[index];
}
