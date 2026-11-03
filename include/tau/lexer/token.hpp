#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include <tau/error/unexpected_token.hpp>

#define TAU_KEYWORDS(X)                                                                              \
    X(Fn, "fn", static_assert(true);)                                                                \
    X(Return, "return", static_assert(true);)                                                        \
                                                                                                     \
    /*X(Let, ;)*/                                                                                    \
    X(Var, "var", static_assert(true);)                                                              \
                                                                                                     \
    /*X(If, static_assert(true);)                                                                    \
    X(Else, static_assert(true);)                                                                    \
                                                                                                   \ \
    X(While, static_assert(true);)                                                                   \
    X(For, static_assert(true);)                                                                     \
    X(Continue, static_assert(true);)                                                                \
    X(Break, ;)*/

#define TAU_SYMBOLS(X)                                                                               \
    X(Semicolon, ';', static_assert(true);)                                                          \
    X(Comma, ',', static_assert(true);)                                                              \
                                                                                                     \
    X(LeftParen, '(', static_assert(true);)                                                          \
    X(RightParen, ')', static_assert(true);)                                                         \
    X(LeftBrace, '{', static_assert(true);)                                                          \
    X(RightBrace, '}', static_assert(true);)                                                         \
    X(LeftBracket, '[', static_assert(true);)                                                        \
    X(RightBracket, ']', static_assert(true);)                                                       \
                                                                                                     \
    X(Equals, '=', static_assert(true);)                                                             \
    /*X(DoubleEquals, static_assert(true);)                                                          \
    X(NotEquals, static_assert(true);)                                                               \
                                                                                                   \ \
    X(LessThan, static_assert(true);)                                                                \
    X(GreaterThan, static_assert(true);)                                                             \
    X(LessThanEquals, static_assert(true);)                                                          \
    X(GreaterThanEquals, static_assert(true);)                                                       \
                                                                                                   \ \
    X(LogicalOr, static_assert(true);)                                                               \
    X(LogicalAnd, static_assert(true);)                                                              \
                                                                                                   \ \
    X(Or, static_assert(true);)                                                                      \
    X(Xor, static_assert(true);)                                                                     \
    X(And, ;)*/                                                                                      \
                                                                                                     \
    X(Plus, '+', static_assert(true);)                                                               \
    X(Minus, '-', static_assert(true);)                                                              \
    X(Asterisk, '*', static_assert(true);)                                                           \
    X(ForwardSlash, '/', static_assert(true);)

#define TAU_TOKENS(X)                                      \
    TAU_KEYWORDS(X)                                        \
    TAU_SYMBOLS(X)                                         \
                                                           \
    X(Type, "type", size_t index;)                         \
    X(Identifier, "identifier", size_t index;)             \
                                                           \
    X(Comment, "comment", std::string_view content;)       \
                                                           \
    X(StringLiteral, "string literal", std::string value;) \
    X(CharacterLiteral, "character literal", char value;)  \
    X(IntegerLiteral, "integer literal", uint64_t value;)  \
                                                           \
    X(Eof, "end of file", static_assert(true);)

namespace lexer::token {
template <typename Derived>
struct Token {
  private:
    Token() = default;

  public:
    friend Derived;

    static constexpr auto description() noexcept {
        return Derived::description();
    }
};

#define TAU_TOKENS_AS_STRUCT(type, lexeme, ...)                          \
    struct type : public Token<type> {                                   \
        __VA_ARGS__                                                      \
                                                                         \
        static constexpr auto description() noexcept { return lexeme; }; \
    };

TAU_TOKENS(TAU_TOKENS_AS_STRUCT)

#undef TAU_TOKENS_AS_STRUCT
}  // namespace lexer::token

namespace lexer::detail {
#define TAU_TOKENS_AS_TUPLE(Token, ...) lexer::token::Token,
using TokenTuple = std::tuple<TAU_TOKENS(TAU_TOKENS_AS_TUPLE) std::monostate>;
#undef TAU_TOKENS_AS_TUPLE

template <typename Is, typename Tuple>
struct TokenBuilder;

template <typename... Tokens, std::size_t... Is>
struct TokenBuilder<std::tuple<Tokens...>, std::index_sequence<Is...>> {
    using type =
        std::variant<std::tuple_element_t<Is, std::tuple<Tokens...>>...>;
};

using TokenVariant = typename TokenBuilder<
    TokenTuple,
    std::make_index_sequence<std::tuple_size_v<TokenTuple> - 1>>::type;
}  // namespace lexer::detail

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
    constexpr auto visit(Visitors&&... visitors) & -> decltype(auto) {
        return std::visit(Overload{std::forward<Visitors>(visitors)...}, *this);
    }

    template <typename... Visitors>
    constexpr auto visit(Visitors&&... visitors) const& -> decltype(auto) {
        return std::visit(Overload{std::forward<Visitors>(visitors)...}, *this);
    }

    template <typename... Visitors>
    constexpr auto visit(Visitors&&... visitors) && -> decltype(auto) {
        return std::visit(Overload{std::forward<Visitors>(visitors)...},
                          std::move(*this));
    }

    template <typename T>
    auto get() const& noexcept(false) -> T {
        try {
            return std::get<T>(*this);
        } catch (std::bad_variant_access const& e) {
            return this->visit([](auto&& arg) -> T {
                const token::Token<std::decay_t<decltype(arg)>>& token = arg;

                throw error::UnexpectedToken(token.description(),
                                             T::description());
            });
        }
    }

    template <typename... T>
    [[nodiscard]]
    constexpr auto is() const& noexcept -> bool {
        return (std::holds_alternative<T>(*this) || ...);
    }

    template <typename... T>
    [[nodiscard]]
    constexpr auto is_not() const& noexcept -> bool {
        return (!std::holds_alternative<T>(*this) && ...);
    }

    template <typename T>
    constexpr auto expect() const& noexcept(false) -> void {
        if (this->is<T>()) {
            return;
        }

        this->visit([](auto&& arg) -> void {
            const token::Token<std::decay_t<decltype(arg)>>& token = arg;

            throw error::UnexpectedToken(token.description(), T::description());
        });
    }
};

class Keywords {
  private:
    template <typename T, typename = void>
    struct IsIter : std::false_type {};

    template <typename T>
    struct IsIter<T,
                  std::void_t<decltype(std::declval<T>().begin()),
                              decltype(std::declval<T>().end())>>
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
    static constexpr auto matches(std::string_view identifier,
                                  const char (&keyword)[N]) -> bool {
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
    static auto lookup(std::string_view identifier) -> std::optional<Token> {
        const uint32_t identifier_hash = hash(identifier);

        switch (identifier_hash) {
#define TAU_KEYWORDS_AS_SWITCH_CASE(type, lexeme, ...) \
    case hash(lexeme): {                               \
        if (matches(identifier, lexeme)) {             \
            return lexer::token::type{};               \
        }                                              \
        break;                                         \
    }

            TAU_KEYWORDS(TAU_KEYWORDS_AS_SWITCH_CASE)
#undef TAU_KEYWORDS_AS_SWITCH_CASE
            default:
                break;
        }

        return std::nullopt;
    }
};
}  // namespace lexer
