#include <sstream>
#include <type_traits>
#include <variant>

#include <tau/ast/ast.hpp>
#include <tau/ast/node.hpp>
#include <tau/lexer/pool.hpp>

namespace ast::node {
using lexer::Pool;

auto Ident::dump(const lexer::Pool& pool,
                 std::stringstream& stream) const noexcept -> void {
    stream << R"({"name":")" << this->name(pool) << R"("})";
}

auto Type::dump(const lexer::Pool& pool,
                std::stringstream& stream) const noexcept -> void {
    stream << R"({"name":")" << this->name(pool) << R"("})";
}

auto String::dump(const lexer::Pool& pool,
                  std::stringstream& stream) const noexcept -> void {
    stream << R"({"value":")" << this->value(pool) << R"("})";
}

auto Character::dump(const Pool& /*pool*/,
                     std::stringstream& stream) const noexcept -> void {
    stream << R"({"value":")" << this->value_ << R"("})";
}

auto Integer::dump(const Pool& /*pool*/,
                   std::stringstream& stream) const noexcept -> void {
    stream << R"({"value":")" << this->value_ << R"("})";
}

auto FunctionCall::dump(const lexer::Pool& pool,
                        std::stringstream& stream) const noexcept -> void {
    stream << R"({"ident":)";
    this->ident_.dump(pool, stream);

    stream << R"(,"params":[)";
    for (size_t i = 0; i < this->params_.size(); i++) {
        this->params_[i].dump(pool, stream);

        if (i != this->params_.size() - 1) {
            stream << ',';
        }
    }
    stream << "]}";
}

auto Assignment::dump(const lexer::Pool& pool,
                      std::stringstream& stream) const noexcept -> void {
    stream << R"({"ident":)";
    this->ident_.dump(pool, stream);

    stream << R"(,"expr":)";
    this->expr_->dump(pool, stream);

    stream << '}';
}

auto Group::dump(const lexer::Pool& pool,
                 std::stringstream& stream) const noexcept -> void {
    stream << R"({"expr":)";
    this->expr_->dump(pool, stream);
    stream << '}';
}

auto Factor::dump(const lexer::Pool& pool,
                  std::stringstream& stream) const noexcept -> void {
    stream << '{';

    std::visit(
        [&pool, &stream](auto&& node) -> void {
            using T = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<T, Ident>) {
                stream << R"("ident":)";
            } else if constexpr (std::is_same_v<T, String>) {
                stream << R"("string":)";
            } else if constexpr (std::is_same_v<T, Character>) {
                stream << R"("character":)";
            } else if constexpr (std::is_same_v<T, Integer>) {
                stream << R"("integer":)";
            } else if constexpr (std::is_same_v<T, FunctionCall>) {
                stream << R"("function_call":)";
            } else if constexpr (std::is_same_v<T, Assignment>) {
                stream << R"("assignment":)";
            } else if constexpr (std::is_same_v<T, Group>) {
                stream << R"("group":)";
            }

            static_cast<const Node<T>&>(node).dump(pool, stream);
        },
        *this);

    stream << '}';
}

auto Mult::dump(const lexer::Pool& pool,
                std::stringstream& stream) const noexcept -> void {
    stream << R"({"lhs":)";
    this->lhs_.dump(pool, stream);

    stream << R"(,"rhs":)";
    this->rhs_->dump(pool, stream);

    stream << '}';
}

auto Div::dump(const lexer::Pool& pool,
               std::stringstream& stream) const noexcept -> void {
    stream << R"({"lhs":)";
    this->lhs_.dump(pool, stream);

    stream << R"(,"rhs":)";
    this->rhs_->dump(pool, stream);

    stream << '}';
}

auto Term::dump(const lexer::Pool& pool,
                std::stringstream& stream) const noexcept -> void {
    stream << '{';

    std::visit(
        [&pool, &stream](auto&& node) -> void {
            using T = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<T, Factor>) {
                stream << R"("factor":)";
            } else if constexpr (std::is_same_v<T, Mult>) {
                stream << R"("mult":)";
            } else if constexpr (std::is_same_v<T, Div>) {
                stream << R"("div":)";
            }

            static_cast<const Node<T>&>(node).dump(pool, stream);
        },
        *this);

    stream << '}';
}

auto Add::dump(const lexer::Pool& pool,
               std::stringstream& stream) const noexcept -> void {
    stream << R"({"lhs":)";
    this->lhs_.dump(pool, stream);

    stream << R"(,"rhs":)";
    this->rhs_->dump(pool, stream);

    stream << '}';
}

auto Sub::dump(const lexer::Pool& pool,
               std::stringstream& stream) const noexcept -> void {
    stream << R"({"lhs":)";
    this->lhs_.dump(pool, stream);

    stream << R"(,"rhs":)";
    this->rhs_->dump(pool, stream);

    stream << '}';
}

auto Expr::dump(const lexer::Pool& pool,
                std::stringstream& stream) const noexcept -> void {
    stream << '{';

    std::visit(
        [&pool, &stream](auto&& node) -> void {
            using T = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<T, Term>) {
                stream << R"("term":)";
            } else if constexpr (std::is_same_v<T, Add>) {
                stream << R"("add":)";
            } else if constexpr (std::is_same_v<T, Sub>) {
                stream << R"("sub":)";
            }

            static_cast<const Node<T>&>(node).dump(pool, stream);
        },
        *this);

    stream << '}';
}

auto Block::dump(const lexer::Pool& pool,
                 std::stringstream& stream) const noexcept -> void {
    stream << R"({"statements":[)";

    for (size_t i = 0; i < this->statements_.size(); i++) {
        this->statements_[i].dump(pool, stream);

        if (i != this->statements_.size() - 1) {
            stream << ',';
        }
    }

    stream << "]}";
}

auto VarDecl::dump(const lexer::Pool& pool,
                   std::stringstream& stream) const noexcept -> void {
    stream << R"({"ident":)";
    this->ident_.dump(pool, stream);

    stream << R"(,"type":)";
    this->type_.dump(pool, stream);

    stream << '}';
}

auto FunctionDecl::dump(const lexer::Pool& pool,
                        std::stringstream& stream) const noexcept -> void {
    stream << R"({"ident":)";
    this->ident_.dump(pool, stream);

    stream << R"(,"params":[)";
    for (size_t i = 0; i < this->params_.size(); i++) {
        stream << R"({"ident":)";
        this->params_[i].ident().dump(pool, stream);

        stream << R"(,"type":)";
        this->params_[i].type().dump(pool, stream);

        stream << '}';

        if (i != this->params_.size() - 1) {
            stream << ',';
        }
    }

    stream << R"(],"return_type":)";
    this->return_type_.dump(pool, stream);

    stream << R"(,"body":)";
    this->body_.dump(pool, stream);

    stream << '}';
}

auto Return::dump(const lexer::Pool& pool,
                  std::stringstream& stream) const noexcept -> void {
    stream << R"({"expr":)";
    this->expr_->dump(pool, stream);
    stream << '}';
}

auto Stmt::dump(const lexer::Pool& pool,
                std::stringstream& stream) const noexcept -> void {
    stream << '{';

    std::visit(
        [&pool, &stream](auto&& node) -> void {
            using T = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<T, Expr>) {
                stream << R"("expr":)";
            } else if constexpr (std::is_same_v<T, Block>) {
                stream << R"("block":)";
            } else if constexpr (std::is_same_v<T, VarDecl>) {
                stream << R"("var_decl":)";
            } else if constexpr (std::is_same_v<T, Return>) {
                stream << R"("return":)";
            }

            static_cast<const Node<T>&>(node).dump(pool, stream);
        },
        *this);

    stream << '}';
}

auto Decl::dump(const lexer::Pool& pool,
                std::stringstream& stream) const noexcept -> void {
    stream << '{';

    std::visit(
        [&pool, &stream](auto&& node) -> void {
            using T = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<T, VarDecl>) {
                stream << R"("var_decl":)";
            } else if constexpr (std::is_same_v<T, FunctionDecl>) {
                stream << R"("function_decl":)";
            } else if constexpr (std::is_same_v<T, Factor>) {
                stream << R"("factor":)";
            }

            static_cast<const Node<T>&>(node).dump(pool, stream);
        },
        *this);

    stream << '}';
}
}  // namespace ast::node

namespace ast {
auto Ast::dump(const lexer::Pool& pool) const noexcept -> std::stringstream {
    std::stringstream stream;

    stream << '[';

    for (size_t i = 0; i < this->nodes.size(); i++) {
        this->nodes[i].dump(pool, stream);

        if (i != this->nodes.size() - 1) {
            stream << ',';
        }
    }

    stream << ']';

    return stream;
}
}  // namespace ast
