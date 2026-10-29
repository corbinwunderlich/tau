#pragma once

#include <utility>

#include <tau/ast/ast.hpp>
#include <tau/ast/pool.hpp>
#include <tau/lexer/lexer.hpp>
#include <tau/parser/error.hpp>

namespace parser {
class Parser {
  public:
    Parser(lexer::Lexer l) : lexer(std::move(l)) {};
    [[nodiscard]] auto parse() && noexcept(false) -> ast::Ast;

    lexer::Lexer lexer;

    ast::pool::IdentPool ident_pool;
    ast::pool::StringPool string_pool;
};
} // namespace parser
