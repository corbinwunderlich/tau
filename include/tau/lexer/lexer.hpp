#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <tau/lexer/pool.hpp>
#include <tau/source/file_source.hpp>
#include <tau/source/source.hpp>

#include "token.hpp"

namespace lexer {
using TokenStream = std::vector<Token>;

class Lexer {
  public:
    Lexer(std::shared_ptr<source::Source> source)
        : source_(std::move(source)) {}

    Lexer(std::string source)
        : Lexer(std::make_shared<source::FileSource>(
              source::FileSource(std::move(source)))) {}

    [[nodiscard]]
    auto lex() noexcept(false) -> TokenStream;

    [[nodiscard]]
    auto source() const noexcept -> std::shared_ptr<source::Source>;

    [[nodiscard]]
    auto pool() const noexcept -> std::shared_ptr<lexer::Pool>;

  private:
    enum class CommentType : uint8_t { SingleLine, MultiLine };

    std::shared_ptr<source::Source> source_;
    std::shared_ptr<lexer::Pool> pool_ =
        std::make_shared<lexer::Pool>(lexer::Pool{});

    size_t pos_ = 0;

    void advance(size_t ahead = 1);
    void advance(std::optional<char>& current, size_t ahead = 1);

    [[nodiscard]]
    auto current() const -> std::optional<char>;

    [[nodiscard]]
    auto peek(size_t ahead = 0) const -> std::optional<char>;

    void skip_whitespace();

    auto escape_character(char character) noexcept(false) -> char;

    auto read_type() -> Token;
    auto read_identifier_or_keyword() -> Token;
    auto read_comment(CommentType type) -> Token;
    auto read_string_literal() noexcept(false) -> std::string;
    auto read_character_literal() noexcept(false) -> char;
    auto read_integer_literal() noexcept(false) -> uint64_t;
    auto read_literal_token() -> Token;

    auto lex_next_token() -> Token;
};
}  // namespace lexer
