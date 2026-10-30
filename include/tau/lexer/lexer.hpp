#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <tau/lexer/pool.hpp>
#include <tau/src/src.hpp>

#include "token.hpp"

namespace lexer {
using TokenStream = std::vector<Token>;

class Lexer {
  public:
    Lexer(src::Sources sources) : srcs(std::make_unique<src::Sources>(std::move(sources))) {}
    Lexer(std::string source) : Lexer(src::Sources(std::move(source))) {}

    [[nodiscard]] auto lex() -> TokenStream;

    [[nodiscard]] auto extract_sources() noexcept -> std::unique_ptr<src::Sources>;
    [[nodiscard]] auto extract_pool() noexcept -> std::unique_ptr<lexer::Pool>;

  private:
    enum class CommentType : uint8_t { SingleLine, MultiLine };

    std::unique_ptr<src::Sources> srcs;
    std::unique_ptr<lexer::Pool> pool = std::make_unique<lexer::Pool>(lexer::Pool{});

    size_t pos = 0;

    void advance(size_t ahead = 1);
    void advance(std::optional<char> &current, size_t ahead = 1);
    [[nodiscard]] auto current() const -> std::optional<char>;
    [[nodiscard]] auto peek(size_t ahead = 0) const -> std::optional<char>;

    void skip_whitespace();

    auto escape_character(char character) -> std::optional<char>;

    auto read_type() -> Token;
    auto read_identifier_or_keyword() -> Token;
    auto read_comment(CommentType type) -> Token;
    auto read_string_literal() -> std::optional<std::string>;
    auto read_character_literal() -> std::optional<char>;
    auto read_integer_literal() -> std::optional<uint64_t>;
    auto read_literal_token() -> Token;

    auto lex_next_token() -> Token;
};
} // namespace lexer
