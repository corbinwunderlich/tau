#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <string_view>
#include <variant>

#include <tau/lexer/lexer.hpp>
#include <tau/lexer/pool.hpp>
#include <tau/lexer/token.hpp>
#include <tau/parser/error.hpp>

namespace parser {
class Parser;
}

namespace ast::node {
struct Term;
struct Expr;
struct Stmt;

class Ident {
   private:
    size_t index;

   public:
    Ident() = default;
    Ident(lexer::token::Identifier identifier) : index(identifier.index) {};

    [[nodiscard]] auto name(const lexer::Pool& pool) const -> std::string_view {
        const std::string_view name = pool.resolve(this->index);

        return name;
    }
};

class Type {
   private:
    size_t index;

   public:
    Type() = default;
    Type(lexer::token::Type type) : index(type.index) {};

    [[nodiscard]] auto name(const lexer::Pool& pool) const -> std::string_view {
        const std::string_view name = pool.resolve(this->index);

        return name;
    }
};

struct String {
    std::string value;
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

    static auto parse(parser::Parser& parser,
                      lexer::token::Identifier identifier) noexcept(false)
        -> FunctionCall;
};

struct Assignment {
    Ident ident;
    std::unique_ptr<Expr> expr;

    static auto parse(parser::Parser& parser,
                      lexer::token::Identifier identifier) noexcept(false)
        -> Assignment;
};

struct Group {
    std::unique_ptr<Expr> expr;
};

struct Factor : std::variant<Ident,
                             String,
                             Character,
                             Integer,
                             std::unique_ptr<FunctionCall>,
                             Assignment,
                             Group> {
    using variant::variant;

    static auto parse(parser::Parser& parser) noexcept(false) -> Factor;
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

    static auto parse(parser::Parser& parser) noexcept(false) -> Term;
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

    static auto parse(parser::Parser& parser) noexcept(false) -> Expr;
};

struct Block {
    std::unique_ptr<Stmt[]> stmts;
    size_t stmt_count;

    static auto parse(parser::Parser& parser) noexcept(false) -> Block;
};

struct VarDecl {
    Ident ident;
    Type type;

    static auto parse(parser::Parser& parser) noexcept(false) -> VarDecl;
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

    static auto parse(parser::Parser& parser) noexcept(false) -> FunctionDecl;
};

struct Return {
    std::unique_ptr<Expr> expr;

    static auto parse(parser::Parser& parser) noexcept(false) -> Return;
};

struct Stmt : std::variant<std::unique_ptr<Expr>, Block, VarDecl, Return> {
    using variant::variant;

    static auto parse(parser::Parser& parser) noexcept(false) -> Stmt;
};

struct Decl : std::variant<VarDecl, std::unique_ptr<FunctionDecl>> {
    using variant::variant;

    static auto parse(parser::Parser& parser) noexcept(false) -> Decl;
};
}  // namespace ast::node
