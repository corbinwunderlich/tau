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

auto StringLiteral::dump(const lexer::Pool& pool,
                         std::stringstream& stream) const noexcept -> void {
    stream << R"({"value":")" << this->value(pool) << R"("})";
}

auto CharacterLiteral::dump(const Pool& /*pool*/,
                            std::stringstream& stream) const noexcept -> void {
    stream << R"({"value":")" << value_ << R"("})";
}

auto IntegerLiteral::dump(const Pool& /*pool*/,
                          std::stringstream& stream) const noexcept -> void {
    stream << R"({"value":")" << value_ << R"("})";
}

auto FunctionCall::dump(const lexer::Pool& pool,
                        std::stringstream& stream) const noexcept -> void {
    stream << R"({"ident":)";
    ident_.dump(pool, stream);

    stream << R"(,"params":[)";
    for (size_t i = 0; i < params_.size(); i++) {
        params_[i].dump(pool, stream);

        if (i != params_.size() - 1) {
            stream << ',';
        }
    }
    stream << "]}";
}

auto Group::dump(const lexer::Pool& pool,
                 std::stringstream& stream) const noexcept -> void {
    stream << R"({"expr":)";
    expr_->dump(pool, stream);
    stream << '}';
}

auto Mult::dump(const lexer::Pool& pool,
                std::stringstream& stream) const noexcept -> void {
    stream << R"({"lhs":)";
    lhs_->dump(pool, stream);

    stream << R"(,"rhs":)";
    rhs_->dump(pool, stream);

    stream << '}';
}

auto Div::dump(const lexer::Pool& pool,
               std::stringstream& stream) const noexcept -> void {
    stream << R"({"lhs":)";
    lhs_->dump(pool, stream);

    stream << R"(,"rhs":)";
    rhs_->dump(pool, stream);

    stream << '}';
}

auto Add::dump(const lexer::Pool& pool,
               std::stringstream& stream) const noexcept -> void {
    stream << R"({"lhs":)";
    lhs_->dump(pool, stream);

    stream << R"(,"rhs":)";
    rhs_->dump(pool, stream);

    stream << '}';
}

auto Sub::dump(const lexer::Pool& pool,
               std::stringstream& stream) const noexcept -> void {
    stream << R"({"lhs":)";
    lhs_->dump(pool, stream);

    stream << R"(,"rhs":)";
    rhs_->dump(pool, stream);

    stream << '}';
}

auto Expr::dump(const lexer::Pool& pool,
                std::stringstream& stream) const noexcept -> void {
    stream << '{';

    std::visit(
        [&pool, &stream](auto&& node) -> void {
            using T = std::decay_t<decltype(node)>;

            if constexpr (std::is_same_v<T, Ident>) {
                stream << R"("ident":)";
            } else if constexpr (std::is_same_v<T, StringLiteral>) {
                stream << R"("string":)";
            } else if constexpr (std::is_same_v<T, CharacterLiteral>) {
                stream << R"("character":)";
            } else if constexpr (std::is_same_v<T, IntegerLiteral>) {
                stream << R"("integer":)";
            } else if constexpr (std::is_same_v<T, FunctionCall>) {
                stream << R"("function_call":)";
            } else if constexpr (std::is_same_v<T, Assignment>) {
                stream << R"("assignment":)";
            } else if constexpr (std::is_same_v<T, Group>) {
                stream << R"("group":)";
            } else if constexpr (std::is_same_v<T, Mult>) {
                stream << R"("mult":)";
            } else if constexpr (std::is_same_v<T, Div>) {
                stream << R"("div":)";
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

    for (size_t i = 0; i < statements_.size(); i++) {
        statements_[i].dump(pool, stream);

        if (i != statements_.size() - 1) {
            stream << ',';
        }
    }

    stream << "]}";
}

auto VarDecl::dump(const lexer::Pool& pool,
                   std::stringstream& stream) const noexcept -> void {
    stream << R"({"ident":)";
    ident_.dump(pool, stream);

    stream << R"(,"type":)";
    type_.dump(pool, stream);

    stream << '}';
}

auto FunctionDecl::dump(const lexer::Pool& pool,
                        std::stringstream& stream) const noexcept -> void {
    stream << R"({"ident":)";
    ident_.dump(pool, stream);

    stream << R"(,"params":[)";
    for (size_t i = 0; i < params_.size(); i++) {
        stream << R"({"ident":)";
        params_[i].ident().dump(pool, stream);

        stream << R"(,"type":)";
        params_[i].type().dump(pool, stream);

        stream << '}';

        if (i != params_.size() - 1) {
            stream << ',';
        }
    }

    stream << R"(],"return_type":)";
    if (return_type_.has_value()) {
        return_type_->dump(pool, stream);
    } else {
        stream << "null";
    }

    stream << R"(,"body":)";
    body_.dump(pool, stream);

    stream << '}';
}

auto Return::dump(const lexer::Pool& pool,
                  std::stringstream& stream) const noexcept -> void {
    stream << R"({"expr":)";
    expr_->dump(pool, stream);
    stream << '}';
}

auto Assignment::dump(const lexer::Pool& pool,
                      std::stringstream& stream) const noexcept -> void {
    stream << R"({"lhs":)";
    lhs_->dump(pool, stream);

    stream << R"(,"rhs":)";
    rhs_->dump(pool, stream);

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
            } else if constexpr (std::is_same_v<T, FunctionDecl>) {
                stream << R"("function_decl":)";
            } else if constexpr (std::is_same_v<T, VarDecl>) {
                stream << R"("var_decl":)";
            } else if constexpr (std::is_same_v<T, Return>) {
                stream << R"("return":)";
            } else if constexpr (std::is_same_v<T, Assignment>) {
                stream << R"("assignment":)";
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
