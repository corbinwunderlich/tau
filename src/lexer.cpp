#include <cctype>
#include <charconv>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

#include <tau/lexer/lexer.hpp>
#include <tau/lexer/token.hpp>

using std::nullopt;
using std::optional;

using lexer::Lexer;
using lexer::Token;

enum class Base : uint8_t { Binary = 2, Octal = 8, Decimal = 10, Hexadecimal = 16 };

inline void Lexer::advance(size_t ahead) { this->pos += ahead; }

inline void Lexer::advance(optional<char> &current, size_t ahead) {
    this->pos += ahead;
    current = this->current();
}

auto Lexer::current() -> optional<char> {
    if (this->pos >= this->src.length()) {
        return nullopt;
    }

    return this->src[this->pos];
}

auto Lexer::peek(size_t ahead) -> optional<char> {
    const size_t index = this->pos + ahead;

    if (index >= this->src.length()) {
        return nullopt;
    }

    return this->src[index];
}

inline void Lexer::skip_whitespace() {
    optional<char> current = this->current();

    while (current.has_value() && (std::isspace(static_cast<unsigned char>(*current)) != 0)) {
        this->advance(current);
    }
}

auto Lexer::escape_character(char character) -> optional<char> {
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

        const auto [_, ec] = std::from_chars(this->src.data() + this->pos,
                                             this->src.data() + this->pos + 2, hex, 16);

        if (ec != std::errc()) {
            return nullopt;
        }

        if (hex > std::numeric_limits<char>::max()) {
            return nullopt;
        }

        this->advance();

        return hex;
    }
    default:
        return nullopt;
    }
}

inline auto Lexer::read_identifier_or_keyword() -> Token {
    const size_t start = this->pos;

    optional<char> current = this->current();

    while (current.has_value() &&
           ((std::isalnum(static_cast<unsigned char>(*current)) != 0) || *current == '_')) {
        this->advance(current);
    }

    const auto identifier = std::string_view(this->src).substr(start, this->pos - start);

    if (const auto keyword = Keywords::lookup(identifier); keyword.has_value()) {
        return *keyword;
    }

    return token::Identifier{identifier};
}

inline auto Lexer::read_comment(Lexer::CommentType type) -> Token {
    const size_t start = this->pos;

    optional<char> current = this->current();

    switch (type) {
    case Lexer::CommentType::SingleLine:
        while (current.has_value() && *current != '\n') {
            this->advance(current);
        }

        break;
    case Lexer::CommentType::MultiLine:
        while (current.has_value() && !(current == '*' && this->peek(1) == '/')) {
            this->advance(current);
        }

        this->advance(2);

        break;
    }

    const auto comment = std::string_view(this->src).substr(start, this->pos - start);

    return token::Comment{comment};
}

inline auto Lexer::read_string_literal() -> optional<std::string_view> {
    this->advance();

    const size_t start = this->pos;

    optional<char> current = this->current();
    bool escaped = false;

    while (current.has_value() && (*current != '"' || escaped)) {
        escaped = current == '\\';

        this->advance(current);
    }

    if (!current.has_value()) {
        return nullopt;
    }

    this->advance();

    return std::string_view(this->src).substr(start, this->pos - start);
}

inline auto Lexer::read_character_literal() -> optional<char> {
    this->advance();

    optional<char> current = this->current();

    if (!current.has_value()) {
        return nullopt;
    }

    if (*current != '\\') {
        const char character = *current;

        if (this->advance(); this->current() != '\'') {
            return nullopt;
        }

        this->advance();

        return character;
    }

    this->advance(current);

    optional<char> character = this->escape_character(*current);

    this->advance(2);

    return character;
}

inline auto Lexer::read_integer_literal() -> optional<uint64_t> {
    optional<char> current = this->current();

    if (!current.has_value()) {
        return nullopt;
    }

    size_t start = this->pos;
    Base base = Base::Decimal;

    if (*current == '0') {
        this->advance(current);

        if (!current.has_value()) {
            return nullopt;
        }

        switch (*current) {
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
            return nullopt;
        }

        this->advance(current);

        start = this->pos;
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

    const auto [_, ec] = std::from_chars(this->src.data() + start, this->src.data() + this->pos,
                                         number, static_cast<uint8_t>(base));

    if (ec != std::errc()) {
        return nullopt;
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
    case '+':
        this->advance();
        return token::Plus{};
    case '-':
        this->advance();
        return token::Minus{};
    case '*':
        this->advance();
        return token::Asterisk{};
    case '/':
        if (peek == '/') {
            return this->read_comment(CommentType::SingleLine);
        }

        if (peek == '*') {
            return this->read_comment(CommentType::MultiLine);
        }

        this->advance();
        return token::ForwardSlash{};
    case '(':
        this->advance();
        return token::LeftParen{};
    case ')':
        this->advance();
        return token::RightParen{};
    case '{':
        this->advance();
        return token::LeftBrace{};
    case '}':
        this->advance();
        return token::RightBrace{};
    case '[':
        this->advance();
        return token::LeftBracket{};
    case ']':
        this->advance();
        return token::RightBracket{};
    case '=':
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
    default:
        return token::Invalid{};
    }
}

auto Lexer::next_token() -> Token {
    this->skip_whitespace();

    optional<char> peek = this->peek();

    if (!peek.has_value()) {
        return token::Eof{};
    }

    char current = *peek;

    if ((std::isalpha(static_cast<unsigned char>(current)) != 0) || current == '_') {
        return this->read_identifier_or_keyword();
    }

    if (std::isdigit(static_cast<unsigned char>(current)) != 0) {
        const optional<uint64_t> number = this->read_integer_literal();

        if (!number.has_value()) {
            return token::Invalid{};
        }

        return token::IntegerLiteral{*number};
    }

    if (current == '"') {
        const optional<std::string_view> string = this->read_string_literal();

        if (!string.has_value()) {
            return token::Invalid{};
        }

        return token::StringLiteral{*string};
    }

    if (current == '\'') {
        const optional<char> character = this->read_character_literal();

        if (!character.has_value()) {
            return token::Invalid{};
        }

        return token::CharacterLiteral{*character};
    }

    return this->read_literal_token();
}
