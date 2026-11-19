#include <cctype>
#include <charconv>
#include <limits>
#include <memory>
#include <optional>
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
#include <tau/source/span.hpp>

using std::nullopt;
using std::optional;

namespace lexer {
auto Lexer::lex() -> TokenStream {
    TokenStream token_stream;

    while (pos_.offset() < source_entry_.text().length()) {
        Token token = this->lex_next_token();

        token_stream.push_back(token);
    }

    token_stream.shrink_to_fit();

    return token_stream;
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

inline void Lexer::advance(size_t ahead) {
    pos_.advance(ahead);
}

inline void Lexer::advance(optional<char>& current, size_t ahead) {
    this->advance(ahead);
    current = this->current();
}

auto Lexer::current() const -> optional<char> {
    return source_entry_.at(pos_);
}

auto Lexer::peek(size_t ahead) const -> optional<char> {
    const source::Pos pos = pos_ + ahead;

    return source_entry_.at(pos);
}

inline void Lexer::skip_whitespace_and_comments() {
    optional<char> current = this->current();

    while (true) {
        if (!current.has_value()) {
            return;
        }

        if (std::isspace(static_cast<unsigned char>(*current)) != 0) {
            this->advance(current);

            continue;
        }

        if (*current != '#') {
            return;
        }

        while (current.has_value() && *current != '\n') {
            this->advance(current);
        }
    }
}

auto Lexer::escape_character(char character) -> char {
    switch (character) {
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
            this->advance();

            char hex = 0;

            const auto [_, ec] = std::from_chars(
                source_entry_.text().data() + pos_.offset(),
                source_entry_.text().data() + pos_.offset() + 2, hex, 16);

            if (ec != std::errc()) {
                throw error::BadEscapeSequence("invalid hex code");
            }

            if (hex > std::numeric_limits<char>::max()) {
                throw error::BadEscapeSequence(
                    "hex code is larger than a char");
            }

            this->advance();

            return hex;
        }
        default:
            throw error::BadEscapeSequence();
    }
}

inline auto Lexer::read_type() -> Token {
    const source::Pos start = pos_;

    optional<char> current = this->current();

    while (current.has_value() &&
           std::isalnum(static_cast<unsigned char>(*current)) != 0) {
        this->advance(current);
    }

    const std::string_view identifier =
        source_entry_.range(start, pos_.offset() - start.offset());

    const size_t index = this->pool_->insert_ident(identifier);

    return token::Type{{}, source::Span{start, pos_ - 1}, index};
}

inline auto Lexer::read_identifier_or_keyword() -> Token {
    const source::Pos start = pos_;

    optional<char> current = this->current();

    while (current.has_value() &&
           std::isalnum(static_cast<unsigned char>(*current)) != 0) {
        this->advance(current);
    }

    const std::string_view identifier =
        source_entry_.range(start, pos_.offset() - start.offset());

    const source::Span span{start, pos_ - 1};

    if (const auto keyword = keyword::lookup(span, identifier);
        keyword.has_value()) {
        return *keyword;
    }

    const size_t index = this->pool_->insert_ident(identifier);

    return token::Identifier{{}, span, index};
}

inline auto Lexer::read_string_literal() -> Token {
    this->advance();

    const source::Pos start = pos_;

    optional<char> current = this->current();
    bool escaped = false;

    while (current.has_value() && (*current != '"' || escaped)) {
        escaped = current == '\\';

        this->advance(current);
    }

    if (!current.has_value()) {
        throw error::UnexpectedToken(token::Eof::description(), '"');
    }

    this->advance();

    const auto string = std::string(
        source_entry_.range(start, pos_.offset() - start.offset() - 1));

    const size_t index = this->pool_->insert_string(string);

    return token::StringLiteral{{}, source::Span{start - 1, pos_ - 1}, index};
}

inline auto Lexer::read_character_literal() -> Token {
    const source::Pos start = pos_;

    this->advance();

    optional<char> current = this->current();

    if (*current == '\'') {
        throw error::EmptyCharacterLiteral();
    }

    if (*current != '\\') {
        this->advance();

        optional<char> next_char = this->current();

        if (!next_char.has_value()) {
            throw error::UnexpectedToken(token::Eof::description(), '\'');
        }

        if (*next_char != '\'') {
            throw error::UnexpectedToken(*next_char, '\'');
        }

        this->advance();

        return token::CharacterLiteral{
            {}, source::Span{start, pos_ - 1}, *current};
    }

    this->advance(current);

    if (!current.has_value()) {
        throw error::BadEscapeSequence();
    }

    char character = this->escape_character(*current);

    this->advance(2);

    return token::CharacterLiteral{
        {}, source::Span{start, pos_ - 1}, character};
}

inline auto Lexer::read_integer_literal() -> Token {
    optional<char> current = this->current();

    const source::Pos start = pos_;

    source::Pos digits_start = pos_;
    Base base = Base::Decimal;

    if (*current == '0') {
        const optional<char> peek = this->peek(1);

        if (!peek.has_value()) {
            return token::IntegerLiteral{{}, source::Span{pos_, pos_}, 0};
        }

        switch (*peek) {
            case 'x':
                base = Base::Hexadecimal;
                break;
            case 'o':
                base = Base::Octal;
                break;
            case 'b':
                base = Base::Binary;
                break;
            default:
                throw error::BadIntegerLiteralPrefix(*peek);
        }

        this->advance(current, 2);

        digits_start = pos_;
    }

    while (current.has_value()) {
        this->advance(current);

        switch (base) {
            case Base::Binary:
                if (*current == '0' || *current == '1') {
                    continue;
                }

                break;
            case Base::Octal:
                if (*current >= '0' && *current <= '7') {
                    continue;
                }

                break;
            case Base::Decimal:
                if (std::isdigit(static_cast<unsigned char>(*current)) != 0) {
                    continue;
                }

                break;
            case Base::Hexadecimal:
                if (std::isxdigit(static_cast<unsigned char>(*current)) != 0) {
                    continue;
                }

                break;
        }

        break;
    }

    uint64_t number;

    const auto [_, ec] =
        std::from_chars(source_entry_.text().data() + digits_start.offset(),
                        source_entry_.text().data() + pos_.offset(), number,
                        static_cast<uint8_t>(base));

    if (ec != std::errc()) {
        throw error::BadIntegerLiteral(base);
    }

    return token::IntegerLiteral{{}, source::Span{start, pos_ - 1}, number};
}

inline auto Lexer::read_literal_token() -> Token {
    const optional<char> current = this->current();

    if (!current.has_value()) {
        return token::Eof{{}, source::Span{pos_}};
    }

    // const optional<char> peek = this->peek(1);

    switch (*current) {
        case token::Semicolon::description():
            this->advance();
            return token::Semicolon{{}, source::Span{pos_ - 1}};
        case token::Comma::description():
            this->advance();
            return token::Comma{{}, source::Span{pos_ - 1}};
        case token::LeftParen::description():
            this->advance();
            return token::LeftParen{{}, source::Span{pos_ - 1}};
        case token::RightParen::description():
            this->advance();
            return token::RightParen{{}, source::Span{pos_ - 1}};
        case token::LeftBrace::description():
            this->advance();
            return token::LeftBrace{{}, source::Span{pos_ - 1}};
        case token::RightBrace::description():
            this->advance();
            return token::RightBrace{{}, source::Span{pos_ - 1}};
        case token::LeftBracket::description():
            this->advance();
            return token::LeftBracket{{}, source::Span{pos_ - 1}};
        case token::RightBracket::description():
            this->advance();
            return token::RightBracket{{}, source::Span{pos_ - 1}};
        case token::Equals::description():
            this->advance();
            return token::Equals{{}, source::Span{pos_ - 1}};
            // if (peek != '=') {
            //     this->advance();
            //     return token::Equals{};
            // }
            //
            // this->advance(2);
            // return token::DoubleEquals{};
        // case '!':
        //     if (peek == '=') {
        //         this->advance();
        //         return token::NotEquals{};
        //     }
        //
        //     this->advance(2);
        //     return token::Invalid{};
        // case '<':
        //     if (peek != '=') {
        //         this->advance();
        //         return token::LessThan{};
        //     }
        //
        //     this->advance(2);
        //     return token::LessThanEquals{};
        // case '>':
        //     if (peek != '=') {
        //         this->advance();
        //         return token::GreaterThan{};
        //     }
        //
        //     this->advance(2);
        //     return token::GreaterThanEquals{};
        // case '|':
        //     if (peek != '|') {
        //         this->advance();
        //         return token::Or{};
        //     }
        //
        //     this->advance(2);
        //     return token::LogicalOr{};
        // case '&':
        //     if (peek != '&') {
        //         this->advance();
        //         return token::And{};
        //     }
        //
        //     this->advance(2);
        //     return token::LogicalAnd{};
        // case '^':
        //     this->advance();
        //     return token::Xor{};
        case token::Plus::description():
            this->advance();
            return token::Plus{{}, source::Span{pos_ - 1}};
        case token::Minus::description():
            this->advance();
            return token::Minus{{}, source::Span{pos_ - 1}};
        case token::Asterisk::description():
            this->advance();
            return token::Asterisk{{}, source::Span{pos_ - 1}};
        case token::ForwardSlash::description():
            this->advance();
            return token::ForwardSlash{{}, source::Span{pos_ - 1}};
        default:
            throw error::InvalidToken(*current);
    }
}

auto Lexer::lex_next_token() -> Token {
    this->skip_whitespace_and_comments();

    optional<char> peek = this->peek();

    if (!peek.has_value()) {
        return token::Eof{{}, source::Span{pos_ + 1}};
    }

    char current = *peek;

    if (std::isalpha(static_cast<unsigned char>(current)) != 0) {
        if (static_cast<bool>(
                std::isupper(static_cast<unsigned char>(current)))) {
            return this->read_type();
        }

        return this->read_identifier_or_keyword();
    }

    if (std::isdigit(static_cast<unsigned char>(current)) != 0) {
        return this->read_integer_literal();
    }

    if (current == '"') {
        return this->read_string_literal();
    }

    if (current == '\'') {
        return this->read_character_literal();
    }

    return this->read_literal_token();
}
}  // namespace lexer
