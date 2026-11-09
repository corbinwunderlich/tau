#include <cstddef>
#include <string>
#include <string_view>

#include <tau/lexer/pool.hpp>

namespace lexer {
auto Pool::insert_ident(std::string_view identifier) noexcept -> size_t {
    const auto existing = this->ident_map_.find(identifier);

    if (existing != this->ident_map_.end()) {
        return existing->second;
    }

    const auto index = this->ident_table_.size();

    this->ident_table_.emplace_back(identifier);
    this->ident_map_.insert({identifier, index});

    return index;
}

auto Pool::resolve_ident(size_t index) const noexcept -> std::string_view {
    return this->ident_table_[index];
}

auto Pool::insert_string(std::string string) noexcept -> size_t {
    const auto index = this->string_table_.size();

    this->string_table_.push_back(std::move(string));

    return index;
}

auto Pool::resolve_string(size_t index) const noexcept -> std::string {
    return this->string_table_[index];
}
}  // namespace lexer
