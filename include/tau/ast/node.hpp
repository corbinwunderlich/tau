#pragma once

#include <cstdint>
#include <cstring>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <tau/lexer/lexer.hpp>
#include <tau/lexer/pool.hpp>
#include <tau/lexer/token.hpp>

namespace parser {
class Parser;
}

namespace ast::node {
class Node {
  public:
    virtual auto dump(const lexer::Pool& pool,
                      std::stringstream& stream) const noexcept -> void = 0;

    virtual ~Node() = default;
};

class Term;
class Expr;
class Stmt;

class Ident : public Node {
  private:
    size_t index_;

  public:
    Ident() = default;
    Ident(lexer::token::Identifier identifier) : index_(identifier.index) {};

    [[nodiscard]]
    auto name(const lexer::Pool& pool) const noexcept -> std::string_view {
        const std::string_view name = pool.resolve(this->index_);

        return name;
    }

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;

    [[nodiscard]]
    auto operator==(const Ident& other) const noexcept -> bool {
        return this->index_ == other.index_;
    };
};

class Type : public Node {
  private:
    size_t index_;

  public:
    Type() = default;
    Type(lexer::token::Type type) : index_(type.index) {};

    [[nodiscard]]
    auto name(const lexer::Pool& pool) const noexcept -> std::string_view {
        const std::string_view name = pool.resolve(this->index_);

        return name;
    }

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;

    [[nodiscard]]
    auto operator==(const Type& other) const noexcept -> bool {
        return this->index_ == other.index_;
    };
};

class String : public Node {
  private:
    std::string value_;

  public:
    String(std::string value) : value_(std::move(value)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Character : public Node {
  private:
    char value_;

  public:
    Character(char value) : value_(value) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Integer : public Node {
  private:
    uint64_t value_;

  public:
    Integer(uint64_t value) : value_(value) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class FunctionCall : public Node {
  private:
    Ident ident_;
    std::vector<Expr> params_;

  public:
    FunctionCall(Ident ident, std::vector<Expr> params)
        : ident_(std::move(ident)), params_(std::move(params)) {}

    [[nodiscard]]
    static auto parse(parser::Parser& parser,
                      lexer::token::Identifier identifier) noexcept(false)
        -> FunctionCall;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Assignment : public Node {
  private:
    Ident ident_;
    std::unique_ptr<Expr> expr_;

  public:
    Assignment(Ident ident, std::unique_ptr<Expr> expr)
        : ident_(std::move(ident)), expr_(std::move(expr)) {}

    [[nodiscard]]
    static auto parse(parser::Parser& parser,
                      lexer::token::Identifier identifier) noexcept(false)
        -> Assignment;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Group : public Node {
  private:
    std::unique_ptr<Expr> expr_;

  public:
    Group(std::unique_ptr<Expr> expr) : expr_(std::move(expr)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Factor : public std::variant<Ident,
                                   String,
                                   Character,
                                   Integer,
                                   FunctionCall,
                                   Assignment,
                                   Group>,
               public Node {
  public:
    using variant::variant;

    [[nodiscard]]
    static auto parse(parser::Parser& parser) noexcept(false) -> Factor;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Mult : public Node {
  private:
    Factor lhs_;
    std::unique_ptr<Term> rhs_;

  public:
    Mult(Factor lhs, std::unique_ptr<Term> rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Div : public Node {
  private:
    Factor lhs_;
    std::unique_ptr<Term> rhs_;

  public:
    Div(Factor lhs, std::unique_ptr<Term> rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Term : public std::variant<Factor, Mult, Div>, public Node {
  public:
    using variant::variant;

    static auto parse(parser::Parser& parser) noexcept(false) -> Term;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Add : public Node {
  private:
    Term lhs_;
    std::unique_ptr<Expr> rhs_;

  public:
    Add(Term lhs, std::unique_ptr<Expr> rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Sub : public Node {
  private:
    Term lhs_;
    std::unique_ptr<Expr> rhs_;

  public:
    Sub(Term lhs, std::unique_ptr<Expr> rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Expr : public std::variant<Term, Add, Sub>, public Node {
  public:
    using variant::variant;

    [[nodiscard]]
    static auto parse(parser::Parser& parser) noexcept(false) -> Expr;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Block : public Node {
  private:
    std::vector<Stmt> statements_;

  public:
    Block(std::vector<Stmt> statements) : statements_(std::move(statements)) {}

    [[nodiscard]]
    static auto parse(parser::Parser& parser) noexcept(false) -> Block;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class VarDecl : public Node {
  private:
    Ident ident_;
    Type type_;

  public:
    VarDecl(Ident ident, Type type)
        : ident_(std::move(ident)), type_(std::move(type)) {}

    [[nodiscard]]
    static auto parse(parser::Parser& parser) noexcept(false) -> VarDecl;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class FunctionDecl : public Node {
  public:
    class Param {
      private:
        Ident ident_;
        Type type_;

      public:
        Param() = default;
        Param(Ident i, Type t) : ident_(std::move(i)), type_(std::move(t)) {};

        [[nodiscard]]
        auto ident() const -> Ident {
            return this->ident_;
        }

        [[nodiscard]]
        auto type() const -> Type {
            return this->type_;
        }

        [[nodiscard]]
        auto operator==(const Param& other) const -> bool {
            return this->ident_ == other.ident_ && this->type_ == other.type_;
        };
    };

    FunctionDecl(Ident ident,
                 std::vector<Param> params,
                 Type return_type,
                 Block body)
        : ident_(std::move(ident)),
          params_(std::move(params)),
          return_type_(std::move(return_type)),
          body_(std::move(body)) {}

    [[nodiscard]]
    static auto parse(parser::Parser& parser) noexcept(false) -> FunctionDecl;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;

  private:
    Ident ident_;
    std::vector<Param> params_;
    Type return_type_;
    Block body_;
};

class Return : public Node {
  private:
    std::unique_ptr<Expr> expr_;

  public:
    Return(std::unique_ptr<Expr> expr) : expr_(std::move(expr)) {}

    [[nodiscard]]
    static auto parse(parser::Parser& parser) noexcept(false) -> Return;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Stmt : public std::variant<Expr, Block, VarDecl, Return>, public Node {
  public:
    using variant::variant;

    [[nodiscard]]
    static auto parse(parser::Parser& parser) noexcept(false) -> Stmt;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};

class Decl : public std::variant<VarDecl, FunctionDecl>, public Node {
  public:
    using variant::variant;

    [[nodiscard]]
    static auto parse(parser::Parser& parser) noexcept(false) -> Decl;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void override;
};
}  // namespace ast::node
