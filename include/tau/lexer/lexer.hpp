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
    auto next_token() -> Token;

  private:
    enum class CommentType : uint8_t { SingleLine, MultiLine };

    std::string src;
    size_t pos = 0;

    void advance(size_t ahead = 1);
    void advance(std::optional<char> &current, size_t ahead = 1);
    auto current() -> std::optional<char>;
    auto peek(size_t ahead = 0) -> std::optional<char>;

    void skip_whitespace();

    auto escape_character(char character) -> std::optional<char>;

    auto read_identifier_or_keyword() -> Token;
    auto read_comment(CommentType type) -> Token;
    auto read_string_literal() -> std::optional<std::string_view>;
    auto read_character_literal() -> std::optional<char>;
    auto read_integer_literal() -> std::optional<uint64_t>;
    auto read_literal_token() -> Token;
};
} // namespace lexer
