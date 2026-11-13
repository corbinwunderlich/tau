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
#include <tau/source/source.hpp>

using std::nullopt;
using std::optional;

namespace lexer {
auto Lexer::lex() -> TokenStream {
    TokenStream token_stream;

    while (pos_ < source_->length()) {
        Token token = this->lex_next_token();

        token_stream.push_back(token);
    }

    token_stream.shrink_to_fit();

    return token_stream;
}

auto Lexer::source() const noexcept -> std::shared_ptr<source::Source> {
    return source_;
}

auto Lexer::pool() const noexcept -> std::shared_ptr<lexer::Pool> {
    return pool_;
}

inline void Lexer::advance(size_t ahead) {
    pos_ += ahead;
}

inline void Lexer::advance(optional<char>& current, size_t ahead) {
    pos_ += ahead;
    current = this->current();
}

auto Lexer::current() const -> optional<char> {
    return source_->at(pos_);
}

auto Lexer::peek(size_t ahead) const -> optional<char> {
    const size_t index = pos_ + ahead;

    return source_->at(index);
}

inline void Lexer::skip_whitespace() {
    optional<char> current = this->current();

    while (current.has_value() &&
           std::isspace(static_cast<unsigned char>(*current)) != 0) {
        this->advance(current);
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
                source_->data() + pos_, source_->data() + pos_ + 2, hex, 16);

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
    const size_t start = pos_;

    optional<char> current = this->current();

    while (current.has_value() &&
           std::isalnum(static_cast<unsigned char>(*current)) != 0) {
        this->advance(current);
    }

    const std::string_view identifier = source_->range(start, pos_ - start);

    const size_t index = this->pool_->insert_ident(identifier);

    return token::Type{{}, index};
}

inline auto Lexer::read_identifier_or_keyword() -> Token {
    const size_t start = pos_;

    optional<char> current = this->current();

    while (current.has_value() &&
           std::isalnum(static_cast<unsigned char>(*current)) != 0) {
        this->advance(current);
    }

    const std::string_view identifier = source_->range(start, pos_ - start);

    if (const auto keyword = keyword::lookup(identifier); keyword.has_value()) {
        return *keyword;
    }

    const size_t index = this->pool_->insert_ident(identifier);

    return token::Identifier{{}, index};
}

inline auto Lexer::read_comment(Lexer::CommentType type) -> Token {
    const size_t start = pos_;

    optional<char> current = this->current();

    switch (type) {
        case CommentType::SingleLine:
            while (current.has_value() && *current != '\n') {
                this->advance(current);
            }

            break;
        case CommentType::MultiLine:
            while (current.has_value() &&
                   !(current == '*' && this->peek(1) == '/')) {
                this->advance(current);
            }

            this->advance(2);

            break;
    }

    const auto comment = source_->range(start, pos_ - start);

    return token::Comment{{}, comment};
}

inline auto Lexer::read_string_literal() -> std::string {
    this->advance();

    const size_t start = pos_;

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

    return std::string(source_->range(start, pos_ - start - 1));
}

inline auto Lexer::read_character_literal() -> char {
    this->advance();

    optional<char> current = this->current();

    if (*current == '\'') {
        throw error::EmptyCharacterLiteral();
    }

    if (*current != '\\') {
        this->advance();

        if (optional<char> next_char = this->current();
            !next_char.has_value() || *next_char != '\'') {
            if (!next_char.has_value()) {
                throw error::UnexpectedToken(token::Eof::description(), '\'');
            }

            throw error::UnexpectedToken(*next_char, '\'');
        }

        this->advance();

        return *current;
    }

    this->advance(current);

    if (!current.has_value()) {
        throw error::BadEscapeSequence();
    }

    char character = this->escape_character(*current);

    this->advance(2);

    return character;
}

inline auto Lexer::read_integer_literal() -> uint64_t {
    optional<char> current = this->current();

    size_t start = pos_;
    Base base = Base::Decimal;

    if (*current == '0') {
        const optional<char> peek = this->peek(1);

        if (!peek.has_value()) {
            return 0;
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

        start = pos_;
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
        std::from_chars(source_->data() + start, source_->data() + pos_, number,
                        static_cast<uint8_t>(base));

    if (ec != std::errc()) {
        throw error::BadIntegerLiteral(base);
    }

    return number;
}

inline auto Lexer::read_literal_token() -> Token {
    const optional<char> current = this->current();

    if (!current.has_value()) {
        return token::Eof{};
    }

    const optional<char> peek = this->peek(1);

    switch (*current) {
        case token::Semicolon::description():
            this->advance();
            return token::Semicolon{};
        case token::Comma::description():
            this->advance();
            return token::Comma{};
        case token::LeftParen::description():
            this->advance();
            return token::LeftParen{};
        case token::RightParen::description():
            this->advance();
            return token::RightParen{};
        case token::LeftBrace::description():
            this->advance();
            return token::LeftBrace{};
        case token::RightBrace::description():
            this->advance();
            return token::RightBrace{};
        case token::LeftBracket::description():
            this->advance();
            return token::LeftBracket{};
        case token::RightBracket::description():
            this->advance();
            return token::RightBracket{};
        case token::Equals::description():
            this->advance();
            return token::Equals{};
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
            return token::Plus{};
        case token::Minus::description():
            this->advance();
            return token::Minus{};
        case token::Asterisk::description():
            this->advance();
            return token::Asterisk{};
        case token::ForwardSlash::description():
            if (peek == '/') {
                return this->read_comment(CommentType::SingleLine);
            }

            if (peek == '*') {
                return this->read_comment(CommentType::MultiLine);
            }

            this->advance();
            return token::ForwardSlash{};
        default:
            throw error::InvalidToken(*current);
    }
}

auto Lexer::lex_next_token() -> Token {
    this->skip_whitespace();

    optional<char> peek = this->peek();

    if (!peek.has_value()) {
        return token::Eof{};
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
        const optional<uint64_t> number = this->read_integer_literal();

        return token::IntegerLiteral{{}, *number};
    }

    if (current == '"') {
        std::string string = this->read_string_literal();

        string.shrink_to_fit();

        const size_t index = this->pool_->insert_string(std::move(string));

        return token::StringLiteral{{}, index};
    }

    if (current == '\'') {
        const char character = this->read_character_literal();

        return token::CharacterLiteral{{}, character};
    }

    return this->read_literal_token();
}
}  // namespace lexer
