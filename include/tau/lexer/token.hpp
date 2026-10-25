#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <type_traits>
#include <variant>

#define KEYWORDS(X)                                                                                  \
    X(Fn, {})                                                                                        \
    X(Return, {})                                                                                    \
                                                                                                     \
    X(Let, {})                                                                                       \
    /*X(Var, {})*/                                                                                   \
                                                                                                     \
    X(Identifier, { std::string_view name; })                                                        \
                                                                                                     \
    X(Comment, { std::string_view content; })                                                        \
                                                                                                     \
    /*X(If, {})                                                                                      \
    X(Else, {})                                                                                      \
                                                                                                   \ \
    X(While, {})                                                                                     \
    X(For, {})                                                                                       \
    X(Continue, {})                                                                                  \
    X(Break, {})*/                                                                                   \
                                                                                                     \
    X(Str, {})                                                                                       \
    X(Char, {})                                                                                      \
                                                                                                     \
    X(Int8, {})                                                                                      \
    X(Uint8, {})                                                                                     \
    X(Int16, {})                                                                                     \
    X(Uint16, {})                                                                                    \
    X(Int32, {})                                                                                     \
    X(Uint32, {})                                                                                    \
    X(Int64, {})                                                                                     \
    X(Uint64, {})                                                                                    \
    X(Usize, {})

#define TOKENS(X)                                                                                    \
    KEYWORDS(X)                                                                                      \
                                                                                                     \
    X(StringLiteral, { std::string_view value; })                                                    \
    X(CharacterLiteral, { char value; })                                                             \
    X(IntegerLiteral, { std::uint64_t value; })                                                      \
                                                                                                     \
    X(LeftParen, {})                                                                                 \
    X(RightParen, {})                                                                                \
    X(LeftBrace, {})                                                                                 \
    X(RightBrace, {})                                                                                \
    X(LeftBracket, {})                                                                               \
    X(RightBracket, {})                                                                              \
                                                                                                     \
    X(Equals, {})                                                                                    \
    /*X(DoubleEquals, {})                                                                            \
    X(NotEquals, {})                                                                                 \
                                                                                                   \ \
    X(LessThan, {})                                                                                  \
    X(GreaterThan, {})                                                                               \
    X(LessThanEquals, {})                                                                            \
    X(GreaterThanEquals, {})                                                                         \
                                                                                                   \ \
    X(LogicalOr, {})                                                                                 \
    X(LogicalAnd, {})                                                                                \
                                                                                                   \ \
    X(Or, {})                                                                                        \
    X(Xor, {})                                                                                       \
    X(And, {})*/                                                                                     \
                                                                                                     \
    X(Plus, {})                                                                                      \
    X(Minus, {})                                                                                     \
    X(Asterisk, {})                                                                                  \
    X(ForwardSlash, {})                                                                              \
                                                                                                     \
    X(Eof, {})                                                                                       \
                                                                                                     \
    X(Invalid, {})

namespace lexer::token {
#define AS_STRUCT(Type, Fields) struct Type Fields;
TOKENS(AS_STRUCT)
#undef AS_STRUCT
} // namespace lexer::token

namespace lexer {
#define AS_VARIANT(Token, ...) lexer::token::Token,
struct Token : std::variant<TOKENS(AS_VARIANT) std::monostate> {
    using variant::variant;
};
#undef AS_VARIANT

class Keywords {
  private:
    static constexpr auto lowercase_character(char c) -> char {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
    }

    template <size_t N>
    static constexpr auto lowercase_string(const char (&str)[N]) -> std::array<char, N - 1> {
        std::array<char, N - 1> data{};

        for (size_t i = 0; i < N - 1; i++) {
            data[i] = lowercase_character(str[i]);
        }

        return data;
    }

    template <typename T, typename = void>
    struct IsIter : std::false_type {};

    template <typename T>
    struct IsIter<
        T, std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>>
        : std::true_type {};

    static constexpr uint32_t fnv_a1_base = 0x811c9dc5;
    static constexpr uint32_t fnv_a1_prime = 0x01000193;

    template <typename T, typename = std::enable_if_t<IsIter<T>::value>>
    static constexpr auto hash(T identifier) -> uint32_t {
        uint32_t hash = fnv_a1_base;

        for (const char byte : identifier) {
            hash ^= static_cast<uint32_t>(byte);
            hash *= fnv_a1_prime;
        }

        return hash;
    }

    template <size_t N>
    static constexpr auto hash_keyword(const char (&keyword)[N]) -> uint32_t {
        return hash(lowercase_string(keyword));
    }

    template <size_t N>
    static constexpr auto matches(std::string_view identifier, const std::array<char, N> keyword)
        -> bool {
        if (identifier.size() != N) {
            return false;
        }

        for (size_t i = 0; i < N; i++) {
            if (identifier[i] == keyword[i]) {
                continue;
            }

            return false;
        }

        return true;
    }

  public:
    static constexpr auto lookup(std::string_view identifier) -> std::optional<Token> {
        const uint32_t identifier_hash = hash(identifier);

        switch (identifier_hash) {
#define AS_SWITCH_CASE(Token, ...)                                                                 \
    case hash_keyword(#Token):                                                                     \
        if (matches(identifier, lowercase_string(#Token))) {                                       \
            return lexer::token::Token{};                                                          \
        }                                                                                          \
        break;

            KEYWORDS(AS_SWITCH_CASE)
#undef AS_SWITCH_CASE
        default:
            break;
        }

        return std::nullopt;
    }
};
} // namespace lexer

#undef KEYWORDS
#undef TOKENS
