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
    Token next_token();

  private:
    enum class CommentType : uint8_t { SingleLine, MultiLine };

    std::string src;
    size_t pos = 0;

    void advance(size_t ahead = 1);
    void advance(std::optional<char> &current, size_t ahead = 1);
    std::optional<char> current();
    std::optional<char> peek(size_t ahead = 0);

    void skip_whitespace();

    std::optional<char> escape_character(char character);

    Token read_identifier_or_keyword();
    Token read_comment(CommentType type);
    std::optional<std::string_view> read_string_literal();
    std::optional<char> read_character_literal();
    std::optional<uint64_t> read_integer_literal();
    Token read_literal_token();
};
} // namespace lexer
