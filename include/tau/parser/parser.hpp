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

    friend class ast::node::FunctionCall;
    friend class ast::node::Assignment;
    friend class ast::node::Factor;
    friend class ast::node::Term;
    friend class ast::node::Expr;
    friend class ast::node::Block;
    friend class ast::node::VarDecl;
    friend class ast::node::FunctionDecl;
    friend class ast::node::Return;
    friend class ast::node::Stmt;
    friend class ast::node::Decl;

  private:
    lexer::TokenStream tokens_;
    std::shared_ptr<source::Source> source_;
    std::shared_ptr<lexer::Pool> pool_;

    size_t pos_ = 0;

    auto next_token() -> lexer::Token;
    auto peek_token(size_t ahead = 0) -> lexer::Token;
};
}  // namespace parser
