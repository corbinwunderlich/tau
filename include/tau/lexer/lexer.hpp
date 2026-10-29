#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "token.hpp"

namespace lexer {
class Lexer {
  public:
    Lexer(std::string s) : src(std::move(s)) {}
    auto peek_token() -> Token;
    auto next_token() -> Token;
    [[nodiscard]] auto completed() const -> bool;

  private:
    enum class CommentType : uint8_t { SingleLine, MultiLine };

    const std::string src;
    size_t pos = 0;

    std::optional<Token> next = std::nullopt;

    void advance(size_t ahead = 1);
    void advance(std::optional<char> &current, size_t ahead = 1);
    [[nodiscard]] auto current() const -> std::optional<char>;
    [[nodiscard]] auto peek(size_t ahead = 0) const -> std::optional<char>;

    void skip_whitespace();

    auto escape_character(char character) -> std::optional<char>;

    auto read_type() -> Token;
    auto read_identifier_or_keyword() -> Token;
    auto read_comment(CommentType type) -> Token;
    auto read_string_literal() -> std::optional<std::string_view>;
    auto read_character_literal() -> std::optional<char>;
    auto read_integer_literal() -> std::optional<uint64_t>;
    auto read_literal_token() -> Token;

    auto lex_next_token() -> Token;
};
} // namespace lexer
