#pragma once

#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>

#include <tau/source/entry.hpp>
#include <tau/source/map.hpp>
#include <tau/source/pos.hpp>

namespace lexer {
class Cursor {
  private:
    const source::Entry& entry_;
    source::Pos pos_ = 0;

  public:
    explicit Cursor(const source::Entry& entry) noexcept;

    [[nodiscard]]
    auto pos() const noexcept -> source::Pos;

    [[nodiscard]]
    auto eof() const noexcept -> bool;

    [[nodiscard]]
    auto peek(size_t ahead = 0) const noexcept -> std::optional<char>;

    auto advance(size_t by = 1) noexcept -> void;

    auto bump() noexcept -> std::optional<char>;

    [[nodiscard]]
    auto match(char expected) noexcept -> bool;

    [[nodiscard]]
    auto match(std::string_view expected) noexcept -> bool;

    auto take_while(const std::function<bool(char)>& predicate) noexcept
        -> std::string_view;

    auto take(size_t count) noexcept -> std::string_view;

    [[nodiscard]]
    auto slice(source::Pos from) const noexcept -> std::string_view;
};
}  // namespace lexer
