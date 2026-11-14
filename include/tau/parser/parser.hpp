#pragma once

#include <cstddef>
#include <memory>
#include <utility>

#include <tau/ast/ast.hpp>
#include <tau/ast/node.hpp>
#include <tau/lexer/lexer.hpp>
#include <tau/lexer/pool.hpp>
#include <tau/source/source.hpp>

namespace parser {
class Parser {
  public:
    Parser(lexer::TokenStream tokens,
           std::shared_ptr<source::Source> source,
           std::shared_ptr<lexer::Pool> pool)
        : tokens_(std::move(tokens)),
          source_(std::move(source)),
          pool_(std::move(pool)) {}
    Parser(lexer::Lexer lexer)
        : Parser(lexer.lex(), lexer.source(), lexer.pool()) {}

    [[nodiscard]]
    auto parse() noexcept(false) -> ast::Ast;

  private:
    lexer::TokenStream tokens_;
    std::shared_ptr<source::Source> source_;
    std::shared_ptr<lexer::Pool> pool_;

    size_t pos_ = 0;

    auto next_token() -> lexer::Token;
    auto peek_token(size_t ahead = 0) -> lexer::Token;

    [[nodiscard]]
    auto parse_function_call(lexer::token::Identifier identifier)
        -> ast::node::FunctionCall;

    [[nodiscard]]
    auto parse_assignment(lexer::token::Identifier identifier)
        -> ast::node::Assignment;

    [[nodiscard]]
    auto parse_factor() -> ast::node::Factor;

    [[nodiscard]]
    auto parse_term() -> ast::node::Term;

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
    auto parse_stmt() -> ast::node::Stmt;

    [[nodiscard]]
    auto parse_decl() -> ast::node::Decl;
};
}  // namespace parser
