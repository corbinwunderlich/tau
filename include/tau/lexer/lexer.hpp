#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

#include "token.hpp"

namespace lexer {
class Lexer {
  public:
    Lexer(std::string src);
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

    const std::unordered_map<std::string_view, Token> keywords = {
        {"fn", lexer::token::Fn{}},
        {"return", lexer::token::Return{}},

        {"if", lexer::token::If{}},
        {"else", lexer::token::Else{}},

        {"while", lexer::token::While{}},
        {"for", lexer::token::For{}},
        {"continue", lexer::token::Continue{}},
        {"break", lexer::token::Break{}},

        {"str", lexer::token::Str{}},
        {"char", lexer::token::Char{}},

        {"int8", lexer::token::Int8{}},
        {"uint8", lexer::token::Uint8{}},
        {"int16", lexer::token::Int16{}},
        {"uint16", lexer::token::Uint16{}},
        {"int32", lexer::token::Int32{}},
        {"uint32", lexer::token::Uint32{}},
        {"int64", lexer::token::Int64{}},
        {"uint64", lexer::token::Uint64{}},
        {"usize", lexer::token::Usize{}},
    };
};
} // namespace lexer
