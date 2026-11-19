#pragma once

#include <cstdint>
#include <ostream>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include <tau/error/unexpected_token.hpp>
#include <tau/source/span.hpp>

#define TAU_KEYWORDS(X)   \
    X(Func, "func", )     \
    X(Return, "return", ) \
    X(Var, "var", )

#define TAU_SYMBOLS(X)     \
    X(Semicolon, ";", )    \
    X(Comma, ",", )        \
    X(LeftParen, "(", )    \
    X(RightParen, ")", )   \
    X(LeftBrace, "{", )    \
    X(RightBrace, "}", )   \
    X(LeftBracket, "[", )  \
    X(RightBracket, "]", ) \
    X(Equals, "=", )       \
    X(Plus, "+", )         \
    X(Minus, "-", )        \
    X(Asterisk, "*", )     \
    X(ForwardSlash, "/", )

#define TAU_LITERALS(X)                                   \
    X(Type, "type", size_t index;)                        \
    X(Identifier, "identifier", size_t index;)            \
                                                          \
    X(StringLiteral, "string literal", size_t index;)     \
    X(CharacterLiteral, "character literal", char value;) \
    X(IntegerLiteral, "integer literal", uint64_t value;)

#define TAU_TOKENS(X) \
    TAU_KEYWORDS(X)   \
    TAU_SYMBOLS(X)    \
    TAU_LITERALS(X)   \
    X(Eof, "end of file", )

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

#define TAU_DEFINE_TOKEN(Name, Description, ...)                           \
    struct Name : public Token<Name> {                                     \
        friend struct Token<Name>;                                         \
                                                                           \
        source::Span span;                                                 \
                                                                           \
        __VA_ARGS__                                                        \
                                                                           \
        static constexpr auto description() noexcept -> std::string_view { \
            return Description;                                            \
        };                                                                 \
    };

TAU_TOKENS(TAU_DEFINE_TOKEN)

#undef TAU_DEFINE_TOKEN
}  // namespace lexer::token

namespace lexer::detail {
#define TAU_TOKEN_VARIANTS(Token, ...) lexer::token::Token,
using TokenTuple = std::tuple<TAU_TOKENS(TAU_TOKEN_VARIANTS) std::monostate>;
#undef TAU_TOKEN_VARIANTS

template <typename, typename>
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

    template <typename... T>
    constexpr auto expect() const& noexcept(false) -> void {
        if (this->is<T...>()) {
            return;
        }

        this->visit([](auto&& arg) -> void {
            const token::Token<std::decay_t<decltype(arg)>>& token = arg;

            throw error::UnexpectedToken(token.description(),
                                         (T::description(), ...));
        });
    }
};

auto operator<<(std::ostream& os, const Token& token) -> std::ostream&;
}  // namespace lexer
