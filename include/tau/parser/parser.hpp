#pragma once

#include <cstddef>
#include <memory>
#include <utility>

#include <tau/ast/ast.hpp>
#include <tau/ast/node.hpp>
#include <tau/lexer/lexer.hpp>
#include <tau/lexer/pool.hpp>
#include <tau/source/map.hpp>
#include <tau/source/pos.hpp>

namespace parser {
class Parser {
  public:
    Parser(lexer::TokenStream tokens,
           std::shared_ptr<source::Map> source_map,
           std::shared_ptr<lexer::Pool> pool,
           source::Id source_id)
        : tokens_(std::move(tokens)),
          source_map_(std::move(source_map)),
          pool_(std::move(pool)),
          source_id_(source_id) {}
    Parser(lexer::Lexer lexer)
        : Parser(lexer.lex(),
                 lexer.source_map(),
                 lexer.pool(),
                 lexer.source_id()) {}

    [[nodiscard]]
    auto parse() noexcept(false) -> ast::Ast;

  private:
    lexer::TokenStream tokens_;
    std::shared_ptr<source::Map> source_map_;
    std::shared_ptr<lexer::Pool> pool_;

    source::Id source_id_;
    size_t pos_ = 0;

    const source::Entry& source_entry_ = source_map_->get(source_id_);

    auto next_token() -> lexer::Token;
    auto peek_token(size_t ahead = 0) -> lexer::Token;

    [[nodiscard]]
    auto parse_function_call(lexer::token::Identifier identifier)
        -> ast::node::FunctionCall;

    [[nodiscard]]
    auto parse_factor() -> ast::node::Expr;

    [[nodiscard]]
    auto parse_term() -> ast::node::Expr;

    [[nodiscard]]
    auto parse_expr() -> ast::node::Expr;

    [[nodiscard]]
    auto parse_block() -> ast::node::Block;

    [[nodiscard]]
    auto parse_var_decl() -> ast::node::VarDecl;

    [[nodiscard]]
    auto parse_function_decl() -> ast::node::FunctionDecl;

    [[nodiscard]]
    auto parse_return() -> ast::node::Return;

    [[nodiscard]]
    auto parse_assignment(std::unique_ptr<ast::node::Expr> lhs)
        -> ast::node::Assignment;

    [[nodiscard]]
    auto parse_stmt() -> ast::node::Stmt;

    [[nodiscard]]
    auto parse_decl() -> ast::node::Decl;
};
}  // namespace parser
