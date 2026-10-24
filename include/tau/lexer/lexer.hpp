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
    Token nextToken();

  private:
    enum class CommentType : uint8_t { SingleLine, Multiline };

    std::string src;
    size_t pos = 0;

    void advance(size_t ahead = 1);
    void advance(std::optional<char> &current, size_t ahead = 1);
    std::optional<char> current();
    std::optional<char> peek(size_t ahead = 0);

    void skipWhitespace();

    std::optional<char> escapeCharacter(char character);

    Token readIdentifierOrKeyword();
    Token readComment(CommentType type);
    std::optional<std::string_view> readStringLiteral();
    std::optional<char> readCharacterLiteral();
    std::optional<uint64_t> readIntegerLiteral();
    Token readLiteralToken();
};
} // namespace lexer
