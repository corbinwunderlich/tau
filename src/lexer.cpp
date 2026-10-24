#include <cctype>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>

#include <tau/lexer/keywords.hpp>
#include <tau/lexer/lexer.hpp>
#include <tau/lexer/token.hpp>

using std::nullopt;
using std::optional;

using lexer::Lexer;
using lexer::Token;

enum class Base : uint8_t { Binary = 2, Octal = 8, Decimal = 10, Hexadecimal = 16 };

Lexer::Lexer(std::string src) : src(std::move(src)) {};

inline void Lexer::advance(size_t ahead) { this->pos += ahead; }

inline void Lexer::advance(optional<char> &current, size_t ahead) {
    this->pos += ahead;
    current = this->current();
}

optional<char> Lexer::current() {
    if (this->pos >= this->src.length()) {
        return nullopt;
    }

    return this->src[this->pos];
}

optional<char> Lexer::peek(size_t ahead) {
    const size_t index = this->pos + ahead;

    if (index >= this->src.length()) {
        return nullopt;
    }

    return this->src[index];
}

inline void Lexer::skipWhitespace() {
    optional<char> current = this->current();

    while (current.has_value() && std::isspace(*current)) {
        this->advance(current);
    }
}

optional<char> Lexer::escapeCharacter(char character) {
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

        uint8_t hex = 0;

        const auto [_, ec] = std::from_chars(this->src.data() + this->pos,
                                             this->src.data() + this->pos + 2, hex, 16);

        if (ec != std::errc()) {
            return nullopt;
        }

        if (hex > 127) {
            return nullopt;
        }

        this->advance();

        return hex;
    }
    default:
        return nullopt;
    }
}

inline Token Lexer::readIdentifierOrKeyword() {
    const size_t start = this->pos;

    optional<char> current = this->current();

    while (current.has_value() && (std::isalnum(*current) || *current == '_')) {
        this->advance(current);
    }

    const auto identifier = std::string_view(this->src).substr(start, this->pos - start);

    if (const auto keyword = Keywords::lookup(identifier); keyword.has_value()) {
        return *keyword;
    }

    return token::Identifier{.name = identifier};
}

inline Token Lexer::readComment(Lexer::CommentType type) {
    const size_t start = this->pos;

    optional<char> current = this->current();

    switch (type) {
    case Lexer::CommentType::SingleLine:
        while (current.has_value() && *current != '\n') {
            this->advance(current);
        }

        break;
    case Lexer::CommentType::Multiline:
        while (current.has_value() && !(current == '*' && this->peek(1) == '/')) {
            this->advance(current);
        }

        this->advance(2);

        break;
    }

    const auto comment = std::string_view(this->src).substr(start, this->pos - start);

    return token::Comment{.content = comment};
}

inline optional<std::string_view> Lexer::readStringLiteral() {
    this->advance();

    const size_t start = this->pos;

    optional<char> current = this->current();
    bool escaped = false;

    while (current.has_value() && (*current != '"' || escaped)) {
        if (current == '\\') {
            escaped = true;
        } else {
            escaped = false;
        }

        this->advance(current);
    }

    if (!current.has_value()) {
        return nullopt;
    }

    this->advance();

    return std::string_view(this->src).substr(start, this->pos - start);
}

inline optional<char> Lexer::readCharacterLiteral() {
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

    optional<char> character = this->escapeCharacter(*current);

    this->advance(2);

    return character;
}

inline optional<uint64_t> Lexer::readIntegerLiteral() {
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
            if (std::isdigit(*current)) {
                continue;
            }

            break;
        case Base::Hexadecimal:
            if (std::isxdigit(*current)) {
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

inline Token Lexer::readLiteralToken() {
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
            return this->readComment(CommentType::SingleLine);
        }

        if (peek == '*') {
            return this->readComment(CommentType::Multiline);
        }

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
        if (peek != '=') {
            this->advance();
            return token::Equals{};
        }

        this->advance(2);
        return token::DoubleEquals{};
    case '!':
        if (peek == '=') {
            this->advance();
            return token::NotEquals{};
        }

        this->advance(2);
        return token::Invalid{};
    case '<':
        if (peek != '=') {
            this->advance();
            return token::LessThan{};
        }

        this->advance(2);
        return token::LessThanEquals{};
    case '>':
        if (peek != '=') {
            this->advance();
            return token::GreaterThan{};
        }

        this->advance(2);
        return token::GreaterThanEquals{};
    case '|':
        if (peek != '|') {
            this->advance();
            return token::Or{};
        }

        this->advance(2);
        return token::LogicalOr{};
    case '&':
        if (peek != '&') {
            this->advance();
            return token::And{};
        }

        this->advance(2);
        return token::LogicalAnd{};
    case '^':
        this->advance();
        return token::Xor{};
    default:
        return token::Invalid{};
    }
}

Token Lexer::nextToken() {
    this->skipWhitespace();

    optional<char> peek = this->peek();

    if (!peek.has_value()) {
        return token::Eof{};
    }

    char current = *peek;

    if (std::isalpha(current) || current == '_') {
        return this->readIdentifierOrKeyword();
    }

    if (std::isdigit(current)) {
        const optional<uint64_t> number = this->readIntegerLiteral();

        if (!number.has_value()) {
            return token::Invalid{};
        }

        return token::IntegerLiteral{.value = *number};
    }

    if (current == '"') {
        const optional<std::string_view> string = this->readStringLiteral();

        if (!string.has_value()) {
            return token::Invalid{};
        }

        return token::StringLiteral{.value = *string};
    }

    if (current == '\'') {
        const optional<char> character = this->readCharacterLiteral();

        if (!character.has_value()) {
            return token::Invalid{};
        }

        return token::CharacterLiteral{.value = *character};
    }

    return this->readLiteralToken();
}
