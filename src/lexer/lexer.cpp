#include <cctype>
#include <charconv>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

#include <tau/error/bad_escape_sequence.hpp>
#include <tau/error/bad_integer_literal.hpp>
#include <tau/error/bad_integer_literal_prefix.hpp>
#include <tau/error/empty_character_literal.hpp>
#include <tau/error/invalid_token.hpp>
#include <tau/error/unexpected_token.hpp>
#include <tau/lexer/base.hpp>
#include <tau/lexer/keyword.hpp>
#include <tau/lexer/lexer.hpp>
#include <tau/lexer/pool.hpp>
#include <tau/lexer/token.hpp>
#include <tau/source/map.hpp>
#include <tau/source/pos.hpp>
#include <tau/source/span.hpp>

#include "symbols.hpp"

namespace lexer {
namespace {
auto is_space(char c) noexcept -> bool {
    return std::isspace(static_cast<unsigned char>(c)) != 0;
}

auto is_word_start(char c) noexcept -> bool {
    return std::isalpha(static_cast<unsigned char>(c)) != 0;
}

auto is_word_continue(char c) noexcept -> bool {
    return std::isalnum(static_cast<unsigned char>(c)) != 0;
}

auto is_upper(char c) noexcept -> bool {
    return std::isupper(static_cast<unsigned char>(c)) != 0;
}

auto is_digit_base(Base base, char c) noexcept -> bool {
    switch (base) {
        case Base::Binary:
            return c == '0' || c == '1';
        case Base::Octal:
            return c >= '0' && c <= '7';
        case Base::Decimal:
            return std::isdigit(static_cast<unsigned char>(c)) != 0;
        case Base::Hexadecimal:
            return std::isxdigit(static_cast<unsigned char>(c)) != 0;
    }

    return false;
}
}  // namespace

auto Lexer::lex() -> TokenStream {
    TokenStream tokens;

    while (true) {
        this->skip_whitespace_and_comments();

        if (cursor_.eof()) {
            break;
        }

        Token token = this->next_token();

        tokens.push_back(token);
    }

    tokens.push_back(token::Eof{{}, source::Span{cursor_.pos()}});

    tokens.shrink_to_fit();

    return tokens;
}

auto Lexer::source_map() const noexcept -> std::shared_ptr<source::Map> {
    return source_map_;
}

auto Lexer::source_id() const noexcept -> source::Id {
    return source_id_;
}

auto Lexer::pool() const noexcept -> std::shared_ptr<lexer::Pool> {
    return pool_;
}

auto Lexer::span_from(source::Pos start) const noexcept -> source::Span {
    return source::Span{start, cursor_.pos() - 1};
}

auto Lexer::skip_whitespace_and_comments() noexcept -> void {
    while (const char character = *cursor_.peek()) {
        if (is_space(character)) {
            cursor_.advance();

            continue;
        }

        if (character != '#') {
            return;
        }

        cursor_.take_while([](char c) -> bool { return c != '\n'; });
    }
}

auto Lexer::read_escape_sequence() -> char {
    const std::optional<char> character = cursor_.bump();

    if (!character.has_value()) {
        throw error::BadEscapeSequence("unterminated escape sequence");
    }

    switch (*character) {
        case '0':
            return '\0';
        case 'a':
            return '\a';
        case 'b':
            return '\b';
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case 'r':
            return '\r';
        case '\\':
            return '\\';
        case '\'':
            return '\'';
        case '"':
            return '"';
        case 'x': {
            const std::string_view digits = cursor_.take(2);

            unsigned char value = 0;

            const auto [_, ec] = std::from_chars(
                digits.data(), digits.data() + digits.size(), value, 16);

            if (digits.size() != 2 || ec != std::errc()) {
                throw error::BadEscapeSequence("invalid hex escape sequence");
            }

            return static_cast<char>(value);
        }
        default:
            throw error::BadEscapeSequence();
    }
}

auto Lexer::read_word() noexcept -> Token {
    const source::Pos start = cursor_.pos();
    const std::string_view word = cursor_.take_while(is_word_continue);
    const source::Span span = this->span_from(start);

    if (is_upper(word.front())) {
        return token::Type{{}, span, pool_->insert_ident(word)};
    }

    if (const std::optional<Token> keyword = keyword::lookup(span, word);
        keyword.has_value()) {
        return *keyword;
    }

    return token::Identifier{{}, span, pool_->insert_ident(word)};
}

auto Lexer::read_symbol() -> Token {
    const source::Pos start = cursor_.pos();

    for (const detail::SymbolEntry& entry : detail::symbols) {
        if (!cursor_.match(entry.lexeme)) {
            continue;
        }

        return entry.make(this->span_from(start));
    }

    throw error::InvalidToken(*cursor_.peek());
}

auto Lexer::read_number() -> Token {
    const source::Pos start = cursor_.pos();

    Base base = Base::Decimal;

    if (cursor_.peek() == '0') {
        switch (cursor_.peek(1).value_or('\0')) {
            case 'x':
            case 'X':
                base = Base::Hexadecimal;
                break;
            case 'o':
            case 'O':
                base = Base::Octal;
                break;
            case 'b':
            case 'B':
                base = Base::Binary;
                break;
            default:
                break;
        }

        if (base != Base::Decimal) {
            cursor_.advance(2);
        }
    }

    const std::string_view digits = cursor_.take_while(
        [base](char c) -> bool { return is_digit_base(base, c); });

    if (digits.empty()) {
        throw error::BadIntegerLiteral(base);
    }

    if (const std::optional<char> next = cursor_.peek();
        next.has_value() && is_word_continue(*next)) {
        cursor_.take_while(is_word_continue);

        throw error::BadIntegerLiteral(base);
    }

    uint64_t value = 0;

    const auto [_, ec] =
        std::from_chars(digits.data(), digits.data() + digits.size(), value,
                        static_cast<uint8_t>(base));

    if (ec != std::errc()) {
        throw error::BadIntegerLiteral(base);
    }

    return token::IntegerLiteral{{}, this->span_from(start), value};
}

auto Lexer::read_string() -> Token {
    const source::Pos start = cursor_.pos();

    cursor_.advance();

    std::string value;

    while (true) {
        const std::optional<char> character = cursor_.bump();

        if (!character.has_value()) {
            throw error::UnexpectedToken(token::Eof::description(), '"');
        }

        if (*character == '"') {
            break;
        }

        if (*character == '\\') {
            value.push_back(this->read_escape_sequence());

            continue;
        }

        value.push_back(*character);
    }

    return token::StringLiteral{
        {}, this->span_from(start), pool_->insert_string(value)};
}

auto Lexer::read_character() -> Token {
    const source::Pos start = cursor_.pos();

    cursor_.advance();

    const std::optional<char> character = cursor_.bump();

    if (!character.has_value()) {
        throw error::UnexpectedToken(token::Eof::description(), '\'');
    }

    if (*character == '\'') {
        throw error::EmptyCharacterLiteral();
    }

    const char value =
        (*character == '\\') ? this->read_escape_sequence() : *character;

    if (!cursor_.match('\'')) {
        std::optional<char> peek = cursor_.peek();

        throw error::UnexpectedToken(peek.has_value()
                                         ? std::string_view{&*peek, 1}
                                         : token::Eof::description(),
                                     "'");
    }

    return token::CharacterLiteral{{}, this->span_from(start), value};
}

auto Lexer::next_token() -> Token {
    char character = *cursor_.peek();

    if (is_word_start(character)) {
        return this->read_word();
    }

    if (is_digit_base(Base::Decimal, character)) {
        return this->read_number();
    }

    switch (character) {
        case '"':
            return this->read_string();
        case '\'':
            return this->read_character();
        default:
            return this->read_symbol();
    }
}
}  // namespace lexer
