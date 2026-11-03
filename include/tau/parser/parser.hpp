#pragma once

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
    Parser(lexer::TokenStream token_stream,
           std::unique_ptr<source::Source> source,
           std::unique_ptr<lexer::Pool> pool)
        : tokens_(std::move(token_stream)),
          source_(std::move(source)),
          pool_(std::move(pool)) {}
    Parser(lexer::Lexer lexer)
        : Parser(lexer.lex(), lexer.extract_source(), lexer.extract_pool()) {}

    [[nodiscard]]
    auto parse() && noexcept(false) -> ast::Ast;

    friend struct ast::node::FunctionCall;
    friend struct ast::node::Assignment;
    friend struct ast::node::Factor;
    friend struct ast::node::Term;
    friend struct ast::node::Expr;
    friend struct ast::node::Block;
    friend struct ast::node::VarDecl;
    friend struct ast::node::FunctionDecl;
    friend struct ast::node::Return;
    friend struct ast::node::Stmt;
    friend struct ast::node::Decl;

  private:
    lexer::TokenStream tokens_;
    std::unique_ptr<source::Source> source_;
    std::unique_ptr<lexer::Pool> pool_;

    size_t pos_ = 0;

    auto next_token() -> lexer::Token;
    auto peek_token() -> lexer::Token;
};
}  // namespace parser
