#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include <tau/parser/error.hpp>

#define BLANK_TOKEN static_assert(true);

#define KEYWORDS(X)                                                                                  \
    X(Fn, "fn", BLANK_TOKEN)                                                                         \
    X(Return, "return", BLANK_TOKEN)                                                                 \
                                                                                                     \
    /*X(Let, BLANK_TOKEN)*/                                                                          \
    X(Var, "var", BLANK_TOKEN)                                                                       \
                                                                                                     \
    /*X(If, BLANK_TOKEN)                                                                             \
    X(Else, BLANK_TOKEN)                                                                             \
                                                                                                   \ \
    X(While, BLANK_TOKEN)                                                                            \
    X(For, BLANK_TOKEN)                                                                              \
    X(Continue, BLANK_TOKEN)                                                                         \
    X(Break, BLANK_TOKEN)*/

#define SYMBOLS(X)                                                                                   \
    X(Semicolon, ';', BLANK_TOKEN)                                                                   \
    X(Comma, ',', BLANK_TOKEN)                                                                       \
                                                                                                     \
    X(LeftParen, '(', BLANK_TOKEN)                                                                   \
    X(RightParen, ')', BLANK_TOKEN)                                                                  \
    X(LeftBrace, '{', BLANK_TOKEN)                                                                   \
    X(RightBrace, '}', BLANK_TOKEN)                                                                  \
    X(LeftBracket, '[', BLANK_TOKEN)                                                                 \
    X(RightBracket, ']', BLANK_TOKEN)                                                                \
                                                                                                     \
    X(Equals, '=', BLANK_TOKEN)                                                                      \
    /*X(DoubleEquals, BLANK_TOKEN)                                                                   \
    X(NotEquals, BLANK_TOKEN)                                                                        \
                                                                                                   \ \
    X(LessThan, BLANK_TOKEN)                                                                         \
    X(GreaterThan, BLANK_TOKEN)                                                                      \
    X(LessThanEquals, BLANK_TOKEN)                                                                   \
    X(GreaterThanEquals, BLANK_TOKEN)                                                                \
                                                                                                   \ \
    X(LogicalOr, BLANK_TOKEN)                                                                        \
    X(LogicalAnd, BLANK_TOKEN)                                                                       \
                                                                                                   \ \
    X(Or, BLANK_TOKEN)                                                                               \
    X(Xor, BLANK_TOKEN)                                                                              \
    X(And, BLANK_TOKEN)*/                                                                            \
                                                                                                     \
    X(Plus, '+', BLANK_TOKEN)                                                                        \
    X(Minus, '-', BLANK_TOKEN)                                                                       \
    X(Asterisk, '*', BLANK_TOKEN)                                                                    \
    X(ForwardSlash, '/', BLANK_TOKEN)

#define TOKENS(X)                                                                                  \
    KEYWORDS(X)                                                                                    \
    SYMBOLS(X)                                                                                     \
                                                                                                   \
    X(Type, "type", std::string_view identifier;)                                                  \
    X(Identifier, "identifier", std::string_view name;)                                            \
                                                                                                   \
    X(Comment, "comment", std::string_view content;)                                               \
                                                                                                   \
    X(StringLiteral, "string literal", std::string_view value;)                                    \
    X(CharacterLiteral, "character literal", char value;)                                          \
    X(IntegerLiteral, "integer literal", std::uint64_t value;)                                     \
                                                                                                   \
    X(Eof, "end of file", BLANK_TOKEN)                                                             \
    X(Invalid, "invalid", BLANK_TOKEN)

namespace lexer::token {
template <typename Derived>
struct Token {
  private:
    Token() = default;

  public:
    friend Derived;

    static constexpr auto text() noexcept { return Derived::text(); }
};

#define AS_STRUCT(Type, Text, ...)                                                                 \
    struct Type : public Token<Type> {                                                             \
        friend struct Token<Type>;                                                                 \
                                                                                                   \
        __VA_ARGS__                                                                                \
                                                                                                   \
        static constexpr auto text() noexcept { return Text; };                                    \
    };
TOKENS(AS_STRUCT)
#undef AS_STRUCT
} // namespace lexer::token

namespace lexer::detail {
#define AS_TUPLE(Token, ...) lexer::token::Token,
using TokenTuple = std::tuple<TOKENS(AS_TUPLE) std::monostate>;
#undef AS_TUPLE

template <typename Is, typename Tuple>
struct TokenBuilder;

template <typename... Tokens, std::size_t... Is>
struct TokenBuilder<std::tuple<Tokens...>, std::index_sequence<Is...>> {
    using type = std::variant<std::tuple_element_t<Is, std::tuple<Tokens...>>...>;
};

using TokenVariant = typename TokenBuilder<
    TokenTuple, std::make_index_sequence<std::tuple_size_v<TokenTuple> - 1>>::type;
} // namespace lexer::detail

namespace lexer {
class Token : public detail::TokenVariant {
  private:
    template <class... Ts>
    struct Overload : Ts... {
        using Ts::operator()...;
    };

    template <class... Ts>
    Overload(Ts...) -> Overload<Ts...>;

  public:
    using variant::variant;

    template <typename... Visitors>
    constexpr auto visit(Visitors &&...visitors) & -> decltype(auto) {
        return std::visit(Overload{std::forward<Visitors>(visitors)...}, *this);
    }

    template <typename... Visitors>
    constexpr auto visit(Visitors &&...visitors) const & -> decltype(auto) {
        return std::visit(Overload{std::forward<Visitors>(visitors)...}, *this);
    }

    template <typename... Visitors>
    constexpr auto visit(Visitors &&...visitors) && -> decltype(auto) {
        return std::visit(Overload{std::forward<Visitors>(visitors)...}, std::move(*this));
    }

    template <typename T>
    auto get() const & noexcept(false) -> T {
        try {
            return std::get<T>(*this);
        } catch (std::bad_variant_access const &e) {
            return this->visit([](auto &&arg) -> T {
                using TokenType = std::decay_t<decltype(arg)>;
                static_assert(std::is_base_of_v<token::Token<TokenType>, TokenType>);

                const token::Token<TokenType> &token = arg;

                throw parser::error::UnexpectedToken(token.text(), T::text());
            });
        }
    }

    template <typename... T>
    [[nodiscard]] constexpr auto is() const & noexcept -> bool {
        return (std::holds_alternative<T>(*this) || ...);
    }

    template <typename... T>
    [[nodiscard]] constexpr auto is_not() const & noexcept -> bool {
        return (!std::holds_alternative<T>(*this) && ...);
    }

    template <typename T>
    constexpr auto expect() const & noexcept(false) -> void {
        if (this->is<T>()) {
            return;
        }

        this->visit([](auto &&arg) -> void {
            using TokenType = std::decay_t<decltype(arg)>;
            static_assert(std::is_base_of_v<token::Token<TokenType>, TokenType>);

            const token::Token<TokenType> &token = arg;

            throw parser::error::UnexpectedToken(token.text(), T::text());
        });
    }
};

class Keywords {
  private:
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
    static constexpr auto hash(const char (&identifier)[N]) -> uint32_t {
        std::array<char, N - 1> data{};

        for (size_t i = 0; i < N - 1; i++) {
            data[i] = identifier[i];
        }

        return hash(data);
    }

    template <size_t N>
    static constexpr auto matches(std::string_view identifier, const char (&keyword)[N]) -> bool {
        if (identifier.size() != N - 1) {
            return false;
        }

        for (size_t i = 0; i < N - 1; i++) {
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
#define AS_SWITCH_CASE(Token, keyword, ...)                                                        \
    case hash(keyword): {                                                                          \
        if (matches(identifier, keyword)) {                                                        \
            return lexer::token::Token{};                                                          \
        }                                                                                          \
        break;                                                                                     \
    }

            KEYWORDS(AS_SWITCH_CASE)
#undef AS_SWITCH_CASE
        default:
            break;
        }

        return std::nullopt;
    }
};
} // namespace lexer

#undef BLANK_TOKEN

#undef KEYWORDS
#undef PRIMITIVES
#undef SYMBOLS

#undef TOKENS
