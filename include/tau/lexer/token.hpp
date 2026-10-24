#pragma once

#include <array>
#include <cstdint>
#include <iterator>
#include <optional>
#include <string_view>
#include <type_traits>
#include <variant>

#define KEYWORDS(X)                                                                                \
    X(Fn, {})                                                                                      \
    X(Return, {})                                                                                  \
                                                                                                   \
    X(Let, {})                                                                                     \
    X(Var, {})                                                                                     \
                                                                                                   \
    X(Identifier, { std::string_view name; })                                                      \
                                                                                                   \
    X(Comment, { std::string_view content; })                                                      \
                                                                                                   \
    X(If, {})                                                                                      \
    X(Else, {})                                                                                    \
                                                                                                   \
    X(While, {})                                                                                   \
    X(For, {})                                                                                     \
    X(Continue, {})                                                                                \
    X(Break, {})                                                                                   \
                                                                                                   \
    X(Str, {})                                                                                     \
    X(Char, {})                                                                                    \
                                                                                                   \
    X(Int8, {})                                                                                    \
    X(Uint8, {})                                                                                   \
    X(Int16, {})                                                                                   \
    X(Uint16, {})                                                                                  \
    X(Int32, {})                                                                                   \
    X(Uint32, {})                                                                                  \
    X(Int64, {})                                                                                   \
    X(Uint64, {})                                                                                  \
    X(Usize, {})

#define TOKENS(X)                                                                                  \
    KEYWORDS(X)                                                                                    \
                                                                                                   \
    X(StringLiteral, { std::string_view value; })                                                  \
    X(CharacterLiteral, { char value; })                                                           \
    X(IntegerLiteral, { std::uint64_t value; })                                                    \
                                                                                                   \
    X(LeftParen, {})                                                                               \
    X(RightParen, {})                                                                              \
    X(LeftBrace, {})                                                                               \
    X(RightBrace, {})                                                                              \
    X(LeftBracket, {})                                                                             \
    X(RightBracket, {})                                                                            \
                                                                                                   \
    X(Equals, {})                                                                                  \
    X(DoubleEquals, {})                                                                            \
    X(NotEquals, {})                                                                               \
                                                                                                   \
    X(LessThan, {})                                                                                \
    X(GreaterThan, {})                                                                             \
    X(LessThanEquals, {})                                                                          \
    X(GreaterThanEquals, {})                                                                       \
                                                                                                   \
    X(LogicalOr, {})                                                                               \
    X(LogicalAnd, {})                                                                              \
                                                                                                   \
    X(Or, {})                                                                                      \
    X(Xor, {})                                                                                     \
    X(And, {})                                                                                     \
                                                                                                   \
    X(Plus, {})                                                                                    \
    X(Minus, {})                                                                                   \
    X(Asterisk, {})                                                                                \
    X(ForwardSlash, {})                                                                            \
                                                                                                   \
    X(Eof, {})                                                                                     \
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
    static constexpr char lowercaseCharacter(char c) {
        return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
    }

    template <size_t N>
    static constexpr std::array<char, N - 1> lowercaseString(const char (&str)[N]) {
        std::array<char, N - 1> data{};

        for (size_t i = 0; i < N - 1; i++) {
            data[i] = lowercaseCharacter(str[i]);
        }

        return data;
    }

    template <typename T, typename = void> struct isIter : std::false_type {};

    template <typename T>
    struct isIter<T, std::void_t<decltype(std::begin(std::declval<T &>())),
                                 decltype(std::end(std::declval<T &>()))>> : std::true_type {};

    template <typename T> static constexpr uint32_t hash(T identifier) {
        static_assert(isIter<T>::value, "keyword (T) must be an iterable string");

        uint32_t hash = 2166136261;

        for (const char c : identifier) {
            hash ^= static_cast<uint32_t>(c);
            hash *= 16777619;
        }

        return hash;
    }

    template <size_t N> static constexpr uint32_t hashKeyword(const char (&keyword)[N]) {
        return hash(lowercaseString(keyword));
    }

    template <size_t N>
    static constexpr bool matches(std::string_view identifier, const std::array<char, N> keyword) {
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
    static constexpr std::optional<Token> lookup(std::string_view identifier) {
        const uint32_t identifierHash = hash(identifier);

        switch (identifierHash) {
#define AS_SWITCH_CASE(Token, ...)                                                                 \
    case hashKeyword(#Token):                                                                      \
        if (matches(identifier, lowercaseString(#Token))) {                                        \
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
