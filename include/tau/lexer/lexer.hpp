#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <tau/lexer/pool.hpp>
#include <tau/source/map.hpp>
#include <tau/source/pos.hpp>

#include "tau/source/entry.hpp"
#include "token.hpp"

namespace lexer {
using TokenStream = std::vector<Token>;

class Lexer {
  public:
    Lexer(std::shared_ptr<source::Map> source_map, source::Id source_id)
        : source_map_(std::move(source_map)), source_id_(source_id) {}

    Lexer(std::string source)
        : Lexer(std::make_shared<source::Map>(source::Map(std::move(source))),
                0) {}

    [[nodiscard]]
    auto lex() noexcept(false) -> TokenStream;

    [[nodiscard]]
    auto source_map() const noexcept -> std::shared_ptr<source::Map>;

    [[nodiscard]]
    auto source_id() const noexcept -> source::Id;

    [[nodiscard]]
    auto pool() const noexcept -> std::shared_ptr<lexer::Pool>;

  private:
    std::shared_ptr<source::Map> source_map_;
    std::shared_ptr<lexer::Pool> pool_ =
        std::make_shared<lexer::Pool>(lexer::Pool{});

    source::Id source_id_;
    source::Pos pos_ = 0;

    const source::Entry& source_entry_ = source_map_->get(source_id_);

    void advance(size_t ahead = 1);
    void advance(std::optional<char>& current, size_t ahead = 1);

    [[nodiscard]]
    auto current() const -> std::optional<char>;

    [[nodiscard]]
    auto peek(size_t ahead = 0) const -> std::optional<char>;

    void skip_whitespace_and_comments();

    auto escape_character(char character) noexcept(false) -> char;

    auto read_type() -> Token;
    auto read_identifier_or_keyword() -> Token;
    auto read_string_literal() noexcept(false) -> std::string;
    auto read_character_literal() noexcept(false) -> char;
    auto read_integer_literal() noexcept(false) -> uint64_t;
    auto read_literal_token() -> Token;

    auto lex_next_token() -> Token;
};
}  // namespace lexer
