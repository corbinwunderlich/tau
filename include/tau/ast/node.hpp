#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <sstream>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <tau/lexer/lexer.hpp>
#include <tau/lexer/pool.hpp>
#include <tau/lexer/token.hpp>

namespace ast::node {
template <typename Derived>
class Node {
  private:
    Node() = default;

  public:
    friend Derived;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void {
        return static_cast<const Derived*>(this)->dump(pool, stream);
    };
};

class Term;
class Expr;
class Stmt;

class Ident : public Node<Ident> {
  private:
    size_t index_;

  public:
    Ident() = default;
    Ident(lexer::token::Identifier identifier) : index_(identifier.index) {};

    [[nodiscard]]
    auto name(const lexer::Pool& pool) const noexcept -> std::string_view {
        const std::string_view name = pool.resolve_ident(this->index_);

        return name;
    }

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;

    [[nodiscard]]
    auto operator==(const Ident& other) const noexcept -> bool {
        return this->index_ == other.index_;
    };
};

class Type : public Node<Type> {
  private:
    size_t index_;

  public:
    Type() = default;
    Type(lexer::token::Type type) : index_(type.index) {};

    [[nodiscard]]
    auto name(const lexer::Pool& pool) const noexcept -> std::string_view {
        const std::string_view name = pool.resolve_ident(this->index_);

        return name;
    }

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;

    [[nodiscard]]
    auto operator==(const Type& other) const noexcept -> bool {
        return this->index_ == other.index_;
    };
};

class String : public Node<String> {
  private:
    size_t index_;

  public:
    String(lexer::token::StringLiteral string) : index_(string.index) {}

    [[nodiscard]]
    auto value(const lexer::Pool& pool) const noexcept -> std::string_view {
        const std::string& string = pool.resolve_string(this->index_);

        return string;
    }

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Character : public Node<Character> {
  private:
    char value_;

  public:
    Character(char value) : value_(value) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Integer : public Node<Integer> {
  private:
    uint64_t value_;

  public:
    Integer(uint64_t value) : value_(value) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class FunctionCall : public Node<FunctionCall> {
  private:
    Ident ident_;
    std::vector<Expr> params_;

  public:
    FunctionCall(Ident ident, std::vector<Expr> params)
        : ident_(ident), params_(std::move(params)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Assignment : public Node<Assignment> {
  private:
    Ident ident_;
    std::unique_ptr<Expr> expr_;

  public:
    Assignment(Ident ident, std::unique_ptr<Expr> expr)
        : ident_(ident), expr_(std::move(expr)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Group : public Node<Group> {
  private:
    std::unique_ptr<Expr> expr_;

  public:
    Group(std::unique_ptr<Expr> expr) : expr_(std::move(expr)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Factor : public std::variant<Ident,
                                   String,
                                   Character,
                                   Integer,
                                   FunctionCall,
                                   Assignment,
                                   Group>,
               public Node<Factor> {
  public:
    using variant::variant;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Mult : public Node<Mult> {
  private:
    Factor lhs_;
    std::unique_ptr<Term> rhs_;

  public:
    Mult(Factor lhs, std::unique_ptr<Term> rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Div : public Node<Div> {
  private:
    Factor lhs_;
    std::unique_ptr<Term> rhs_;

  public:
    Div(Factor lhs, std::unique_ptr<Term> rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Term : public std::variant<Factor, Mult, Div>, public Node<Term> {
  public:
    using variant::variant;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Add : public Node<Add> {
  private:
    Term lhs_;
    std::unique_ptr<Expr> rhs_;

  public:
    Add(Term lhs, std::unique_ptr<Expr> rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Sub : public Node<Sub> {
  private:
    Term lhs_;
    std::unique_ptr<Expr> rhs_;

  public:
    Sub(Term lhs, std::unique_ptr<Expr> rhs)
        : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Expr : public std::variant<Term, Add, Sub>, public Node<Expr> {
  public:
    using variant::variant;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Block : public Node<Block> {
  private:
    std::vector<Stmt> statements_;

  public:
    Block(std::vector<Stmt> statements) : statements_(std::move(statements)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class VarDecl : public Node<VarDecl> {
  private:
    Ident ident_;
    Type type_;

  public:
    VarDecl(Ident ident, Type type) : ident_(ident), type_(type) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class FunctionDecl : public Node<FunctionDecl> {
  public:
    class Param {
      private:
        Ident ident_;
        Type type_;

      public:
        Param() = default;
        Param(Ident i, Type t) : ident_(i), type_(t) {};

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
        : ident_(ident),
          params_(std::move(params)),
          return_type_(return_type),
          body_(std::move(body)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;

  private:
    Ident ident_;
    std::vector<Param> params_;
    Type return_type_;
    Block body_;
};

class Return : public Node<Return> {
  private:
    std::unique_ptr<Expr> expr_;

  public:
    Return(std::unique_ptr<Expr> expr) : expr_(std::move(expr)) {}

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Stmt : public std::variant<Expr, Block, VarDecl, Return>,
             public Node<Stmt> {
  public:
    using variant::variant;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};

class Decl : public std::variant<VarDecl, FunctionDecl, Factor>,
             public Node<Decl> {
  public:
    using variant::variant;

    auto dump(const lexer::Pool& pool, std::stringstream& stream) const noexcept
        -> void;
};
}  // namespace ast::node
