#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>

#include <tau/lexer/cursor.hpp>
#include <tau/source/map.hpp>
#include <tau/source/pos.hpp>

namespace lexer {
Cursor::Cursor(const source::Entry& entry) noexcept : entry_(entry) {}

auto Cursor::pos() const noexcept -> source::Pos {
    return pos_;
}

auto Cursor::eof() const noexcept -> bool {
    return pos_.offset() >= entry_.text().length();
}

auto Cursor::peek(size_t ahead) const noexcept -> std::optional<char> {
    const source::Pos pos = pos_ + ahead;

    if (pos.offset() >= entry_.text().length()) {
        return std::nullopt;
    }

    return entry_.at(pos);
}

auto Cursor::advance(size_t by) noexcept -> void {
    pos_.advance(by);
}

auto Cursor::bump() noexcept -> std::optional<char> {
    const std::optional<char> peek = this->peek();

    if (peek.has_value()) {
        this->advance();
    }

    return peek;
}

auto Cursor::match(char expected) noexcept -> bool {
    if (this->peek() != expected) {
        return false;
    }

    this->advance();

    return true;
}

auto Cursor::match(std::string_view expected) noexcept -> bool {
    if (entry_.text().compare(pos_.offset(), expected.length(), expected) !=
        0) {
        return false;
    }

    this->advance(expected.length());

    return true;
}

auto Cursor::take_while(const std::function<bool(char)>& predicate) noexcept
    -> std::string_view {
    const source::Pos start = pos_;

    while (const std::optional<char> character = this->peek()) {
        if (!predicate(*character)) {
            break;
        }

        this->advance();
    }

    return this->slice(start);
}

auto Cursor::take(size_t count) noexcept -> std::string_view {
    const source::Pos start = pos_;

    this->advance(count);

    return this->slice(start);
}

auto Cursor::slice(source::Pos from) const noexcept -> std::string_view {
    return entry_.text().substr(from.offset(), pos_.offset() - from.offset());
}
}  // namespace lexer
