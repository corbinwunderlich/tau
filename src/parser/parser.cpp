#include <cstddef>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include <tau/ast/node.hpp>
#include <tau/error/too_many_parameters.hpp>
#include <tau/lexer/token.hpp>
#include <tau/parser/parser.hpp>

namespace node = ast::node;
using lexer::Token;
namespace token = lexer::token;

namespace parser {
auto Parser::next_token() -> lexer::Token {
    if (pos_ >= tokens_.size()) {
        return lexer::token::Eof{};
    }

    lexer::Token token = tokens_[pos_];

    pos_++;

    return token;
}

auto Parser::peek_token(size_t ahead) -> lexer::Token {
    const size_t index = pos_ + ahead;

    if (index >= tokens_.size()) {
        return lexer::token::Eof{};
    }

    return tokens_[index];
}

auto Parser::parse_function_call(token::Identifier identifier)
    -> node::FunctionCall {
    this->next_token().expect<token::LeftParen>();

    std::vector<node::Expr> params;
    bool has_params = this->peek_token().is_not<token::RightParen>();

    while (has_params) {
        node::Expr expression = this->parse_expr();

        params.push_back(std::move(expression));

        const Token next_token = this->next_token();
        next_token.expect<token::RightParen, token::Comma>();

        if (next_token.is<token::RightParen>()) {
            break;
        }
    }

    if (has_params) {
        params.shrink_to_fit();
    } else {
        this->next_token().expect<token::RightParen>();
    }

    return node::FunctionCall{node::Ident(identifier), std::move(params)};
}

auto Parser::parse_assignment(token::Identifier identifier)
    -> node::Assignment {
    this->next_token().expect<token::Equals>();

    return node::Assignment{node::Ident(identifier),
                            std::make_unique<node::Expr>(this->parse_expr())};
}

auto Parser::parse_factor() -> node::Factor {
    const Token token = this->peek_token();

    node::Factor factor = token.visit(
        [this](token::Identifier identifier) -> node::Factor {
            this->next_token();

            return this->peek_token().visit(
                [this, &identifier](token::LeftParen) -> node::Factor {
                    return this->parse_function_call(identifier);
                },
                [this, &identifier](token::Equals) -> node::Factor {
                    return this->parse_assignment(identifier);
                },
                [&identifier](auto&&) -> node::Factor {
                    return node::Ident(identifier);
                });
        },
        [this](token::StringLiteral str) -> node::Factor {
            this->next_token();

            return node::String{str};
        },
        [this](token::CharacterLiteral character) -> node::Factor {
            this->next_token();

            return node::Character{character.value};
        },
        [this](token::IntegerLiteral integer) -> node::Factor {
            this->next_token();

            return node::Integer{integer.value};
        },
        [this](token::LeftParen) -> node::Factor {
            this->next_token();

            auto expression = std::make_unique<node::Expr>(this->parse_expr());

            this->next_token().expect<token::RightParen>();

            return node::Group{std::move(expression)};
        },
        [](auto&& arg) -> node::Factor {
            const token::Token<std::decay_t<decltype(arg)>>& token = arg;

            throw error::UnexpectedToken(token.description(), "expression");
        });

    return factor;
}

auto Parser::parse_term() -> node::Term {
    node::Factor factor = this->parse_factor();

    const Token token = this->peek_token();

    if (token.is_not<token::Asterisk, token::ForwardSlash>()) {
        return factor;
    }

    const Token operation = this->next_token();

    node::Factor lhs = std::move(factor);
    auto rhs = std::make_unique<node::Term>(this->parse_term());

    node::Term term = operation.visit(
        [&lhs, &rhs](token::Asterisk) -> node::Term {
            return node::Mult{std::move(lhs), std::move(rhs)};
        },
        [&lhs, &rhs](token::ForwardSlash) -> node::Term {
            return node::Div{std::move(lhs), std::move(rhs)};
        },
        [](auto&& arg) -> node::Term {
            const token::Token<std::decay_t<decltype(arg)>>& token = arg;

            throw error::UnexpectedToken(token.description(),
                                         token::Asterisk::description(),
                                         token::ForwardSlash::description());
        });

    return term;
}

auto Parser::parse_expr() -> node::Expr {
    node::Term term = this->parse_term();

    const Token token = this->peek_token();

    if (token.is_not<token::Plus, token::Minus>()) {
        return term;
    }

    const Token operation = this->next_token();

    node::Term lhs = std::move(term);
    auto rhs = std::make_unique<node::Expr>(this->parse_expr());

    node::Expr expression = operation.visit(
        [&lhs, &rhs](token::Plus) -> node::Expr {
            return node::Add{std::move(lhs), std::move(rhs)};
        },
        [&lhs, &rhs](token::Minus) -> node::Expr {
            return node::Sub{std::move(lhs), std::move(rhs)};
        },
        [](auto&& arg) -> node::Expr {
            const token::Token<std::decay_t<decltype(arg)>>& token = arg;

            throw error::UnexpectedToken(token.description(),
                                         token::Plus::description(),
                                         token::Minus::description());
        });

    return expression;
}

auto Parser::parse_block() -> node::Block {
    this->next_token().expect<token::LeftBrace>();

    std::vector<node::Stmt> statements;

    while (this->peek_token().is_not<token::RightBrace, token::Eof>()) {
        node::Stmt statement = this->parse_stmt();

        statements.push_back(std::move(statement));
    }

    this->next_token().expect<token::RightBrace>();

    return node::Block{std::move(statements)};
}

auto Parser::parse_var_decl() -> node::VarDecl {
    this->next_token().expect<token::Var>();

    const auto ident = this->next_token().get<token::Identifier>();
    const auto type = this->next_token().get<token::Type>();

    this->next_token().expect<token::Semicolon>();

    return node::VarDecl{node::Ident(ident), node::Type(type)};
}

auto Parser::parse_function_decl() -> node::FunctionDecl {
    this->next_token().expect<token::Func>();

    const auto ident = this->next_token().get<token::Identifier>();

    this->next_token().expect<token::LeftParen>();

    std::vector<node::FunctionDecl::Param> params;

    while (true) {
        const Token token = this->next_token();

        if (token.is<token::RightParen>()) {
            break;
        }

        const auto param_ident = token.get<token::Identifier>();
        const auto param_type = this->next_token().get<token::Type>();

        bool is_last_param = this->next_token().visit(
            [](token::RightParen) -> bool { return true; },
            [](token::Comma) -> bool { return false; },
            [](auto&& arg) -> bool {
                const token::Token<std::decay_t<decltype(arg)>>& token = arg;

                throw error::UnexpectedToken(token.description(),
                                             token::RightParen::description(),
                                             token::Comma::description());
            });

        params.emplace_back(node::Ident(param_ident), node::Type(param_type));

        if (params.size() > error::TooManyParameters::max_parameters()) {
            throw error::TooManyParameters(
                this->pool_->resolve_ident(ident.index));
        }

        if (is_last_param) {
            break;
        }
    }

    params.shrink_to_fit();

    const auto return_type = this->next_token().get<token::Type>();

    return node::FunctionDecl{node::Ident(ident), std::move(params),
                              node::Type(return_type), this->parse_block()};
}

auto Parser::parse_return() -> node::Return {
    this->next_token().expect<token::Return>();

    auto expression = std::make_unique<node::Expr>(this->parse_expr());

    this->next_token().expect<token::Semicolon>();

    return node::Return{std::move(expression)};
}

auto Parser::parse_stmt() -> node::Stmt {
    const Token token = this->peek_token();

    node::Stmt statement = token.visit(
        [this](token::LeftBrace) -> node::Stmt { return this->parse_block(); },
        [this](token::Var) -> node::Stmt { return this->parse_var_decl(); },
        [this](token::Return) -> node::Stmt { return this->parse_return(); },
        [this](auto&& arg) -> node::Stmt {
            try {
                node::Expr expr = this->parse_expr();

                this->next_token().expect<token::Semicolon>();

                return expr;
            } catch (...) {
                const token::Token<std::decay_t<decltype(arg)>>& token = arg;

                throw error::UnexpectedToken(token.description(), "statement");
            }
        });

    return statement;
}

auto Parser::parse_decl() -> node::Decl {
    const Token token = this->peek_token();

    node::Decl decl = token.visit(
        [this](token::Var) -> node::Decl { return this->parse_var_decl(); },
        [this](token::Func) -> node::Decl {
            return this->parse_function_decl();
        },
        [this](auto&&) -> node::Decl {
            node::Factor factor = this->parse_factor();

            this->next_token().expect<token::Semicolon>();

            return factor;
        });

    return decl;
}

auto Parser::parse() -> ast::Ast {
    std::vector<node::Decl> nodes;

    while (pos_ < tokens_.size()) {
        nodes.push_back(this->parse_decl());
    }

    nodes.shrink_to_fit();

    return {std::move(nodes)};
}
}  // namespace parser
