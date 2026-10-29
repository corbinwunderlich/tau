#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <string_view>
#include <variant>

#include <tau/lexer/lexer.hpp>
#include <tau/lexer/token.hpp>
#include <tau/parser/error.hpp>

#include "pool.hpp"

namespace parser {
class Parser;
}

namespace ast::node {
struct Term;
struct Expr;
struct Stmt;

class Ident {
  private:
    size_t pool_index;

  public:
    Ident() = default;
    Ident(pool::IdentPool &pool, std::string_view s) : pool_index(pool.insert(s)) {}
    Ident(pool::IdentPool &pool, lexer::token::Identifier ident) : Ident(pool, ident.name) {};

    [[nodiscard]] auto name(const pool::IdentPool &pool) const -> std::string_view {
        const pool::Node &node = pool.get(this->pool_index);

        return node.to_string();
    }
};

class Type {
  private:
    size_t pool_index;

  public:
    Type() = default;
    Type(pool::IdentPool &pool, std::string_view s) : pool_index(pool.insert(s)) {}
    Type(pool::IdentPool &pool, lexer::token::Type type) : Type(pool, type.identifier) {};

    [[nodiscard]] auto name(const pool::IdentPool &pool) const -> std::string_view {
        const pool::Node &node = pool.get(this->pool_index);

        return node.to_string();
    }
};

class String {
  private:
    size_t pool_index;

  public:
    String(pool::StringPool &pool, std::string_view s) : pool_index(pool.insert(s)) {};
    String(pool::StringPool &pool, lexer::token::StringLiteral s) : String(pool, s.value) {};

    [[nodiscard]] auto value(const pool::StringPool &pool) const -> std::string_view {
        const pool::Node &node = pool.get(this->pool_index);

        return node.to_string();
    }
};

struct Character {
    char value;
};

struct Integer {
    uint64_t value;
};

struct FunctionCall {
    Ident ident;

    std::unique_ptr<Expr[]> params;
    uint8_t param_count;

    static auto parse(parser::Parser &parser, lexer::token::Identifier identifier) noexcept(false)
        -> FunctionCall;
};

struct Assignment {
    Ident ident;
    std::unique_ptr<Expr> expr;

    static auto parse(parser::Parser &parser, lexer::token::Identifier identifier) noexcept(false)
        -> Assignment;
};

struct Group {
    std::unique_ptr<Expr> expr;
};

struct Factor : std::variant<Ident, String, Character, Integer, std::unique_ptr<FunctionCall>,
                             Assignment, Group> {
    using variant::variant;

    static auto parse(parser::Parser &parser) noexcept(false) -> Factor;
};

struct Mult {
    std::unique_ptr<Factor> lhs;
    std::unique_ptr<Term> rhs;
};

struct Div {
    std::unique_ptr<Factor> lhs;
    std::unique_ptr<Term> rhs;
};

struct Term : std::variant<Factor, Mult, Div> {
    using variant::variant;

    static auto parse(parser::Parser &parser) noexcept(false) -> Term;
};

struct Add {
    std::unique_ptr<Term> lhs;
    std::unique_ptr<Expr> rhs;
};

struct Sub {
    std::unique_ptr<Term> lhs;
    std::unique_ptr<Expr> rhs;
};

struct Expr : std::variant<Term, Add, Sub> {
    using variant::variant;

    static auto parse(parser::Parser &parser) noexcept(false) -> Expr;
};

struct Block {
    std::unique_ptr<Stmt[]> stmts;
    size_t stmt_count;

    static auto parse(parser::Parser &parser) noexcept(false) -> Block;
};

struct VarDecl {
    Ident ident;
    Type type;

    static auto parse(parser::Parser &parser) noexcept(false) -> VarDecl;
};

struct FunctionDecl {
    Ident ident;

    struct Param {
        Ident ident;
        Type type;

        Param() = default;
        Param(Ident i, Type t) : ident(i), type(t) {};
    };

    std::unique_ptr<Param[]> params;
    uint8_t param_count;

    Type return_type;

    Block body;

    static auto parse(parser::Parser &parser) noexcept(false) -> FunctionDecl;
};

struct Return {
    std::unique_ptr<Expr> expr;

    static auto parse(parser::Parser &parser) noexcept(false) -> Return;
};

struct Stmt : std::variant<std::unique_ptr<Expr>, Block, VarDecl, Return> {
    using variant::variant;

    static auto parse(parser::Parser &parser) noexcept(false) -> Stmt;
};

struct Decl : std::variant<VarDecl, std::unique_ptr<FunctionDecl>> {
    using variant::variant;

    static auto parse(parser::Parser &parser) noexcept(false) -> Decl;
};
} // namespace ast::node
