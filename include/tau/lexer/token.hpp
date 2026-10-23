#pragma once

#include <cstdint>
#include <string_view>
#include <variant>

namespace lexer::token {
struct Fn {};
struct Return {};

struct Let {};
struct Var {};

struct Identifier {
    std::string_view name;
};

struct Comment {
    std::string_view content;
};

struct If {};
struct Else {};

struct While {};
struct For {};
struct Continue {};
struct Break {};

struct Str {};
struct Char {};

struct Int8 {};
struct Uint8 {};
struct Int16 {};
struct Uint16 {};
struct Int32 {};
struct Uint32 {};
struct Int64 {};
struct Uint64 {};
struct Usize {};

struct StringLiteral {
    std::string_view value;
};
struct CharacterLiteral {
    char value;
};
struct IntegerLiteral {
    std::uint64_t value;
};

struct LeftParen {};
struct RightParen {};
struct LeftBrace {};
struct RightBrace {};
struct LeftBracket {};
struct RightBracket {};

struct Equals {};
struct DoubleEquals {};
struct NotEquals {};

struct LessThan {};
struct GreaterThan {};
struct LessThanEquals {};
struct GreaterThanEquals {};

struct LogicalOr {};
struct LogicalAnd {};

struct Or {};
struct Xor {};
struct And {};

struct Plus {};
struct Minus {};
struct Asterisk {};
struct ForwardSlash {};

struct Eof {};

struct Invalid {};
} // namespace lexer::token

namespace lexer {
using namespace lexer::token;

using Token =
    std::variant<Fn, Return, Let, Var, Identifier, Comment, If, Else, While, For, Continue, Break,
                 Str, Char, Int8, Uint8, Int16, Uint16, Int32, Uint32, Int64, Uint64, Usize,
                 StringLiteral, CharacterLiteral, IntegerLiteral, LeftParen, RightParen, LeftBrace,
                 RightBrace, LeftBracket, RightBracket, Equals, DoubleEquals, NotEquals, LessThan,
                 GreaterThan, LessThanEquals, GreaterThanEquals, LogicalOr, LogicalAnd, Or, Xor,
                 And, Plus, Minus, Asterisk, ForwardSlash, Eof, Invalid>;
} // namespace lexer
