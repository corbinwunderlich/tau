#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <tau/source/entry.hpp>
#include <tau/source/map.hpp>
#include <tau/source/pos.hpp>

#include "cursor.hpp"
#include "pool.hpp"
#include "token.hpp"

namespace lexer {
struct TokenStream : public std::vector<Token> {};

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
    auto pool() const noexcept -> std::shared_ptr<lexer::Pool>;

    [[nodiscard]]
    auto source_map() const noexcept -> std::shared_ptr<source::Map>;

    [[nodiscard]]
    auto source_id() const noexcept -> source::Id;

  private:
    std::shared_ptr<source::Map> source_map_;
    std::shared_ptr<lexer::Pool> pool_ =
        std::make_shared<lexer::Pool>(lexer::Pool{});

    source::Id source_id_;
    const source::Entry& source_entry_ = source_map_->get(source_id_);

    Cursor cursor_{source_entry_};

    [[nodiscard]]
    auto span_from(source::Pos start) const noexcept -> source::Span;

    auto skip_whitespace_and_comments() noexcept -> void;

    auto read_escape_sequence() noexcept(false) -> char;
    auto read_word() noexcept -> Token;
    auto read_symbol() noexcept(false) -> Token;
    auto read_number() noexcept(false) -> Token;
    auto read_string() noexcept(false) -> Token;
    auto read_character() noexcept(false) -> Token;

    auto next_token() noexcept(false) -> Token;
};
}  // namespace lexer
