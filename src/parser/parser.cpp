#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include <tau/ast/node.hpp>
#include <tau/lexer/token.hpp>
#include <tau/parser/error.hpp>
#include <tau/parser/parser.hpp>

namespace node = ast::node;
using lexer::Token;
namespace token = lexer::token;
using parser::Parser;
namespace error = parser::error;

auto Parser::next_token() -> lexer::Token {
    this->pos++;

    if (this->pos >= this->tokens.size()) {
        return lexer::token::Eof{};
    }

    return this->tokens[this->pos];
}

auto Parser::peek_token() -> lexer::Token {
    const size_t index = this->pos + 1;

    if (index >= this->tokens.size()) {
        return lexer::token::Eof{};
    }

    return this->tokens[index];
}

auto node::FunctionCall::parse(Parser& parser, token::Identifier identifier)
    -> FunctionCall {
    parser.next_token().expect<token::LeftParen>();

    auto params = std::vector<Expr>();

    while (true) {
        const Token token = parser.next_token();

        if (token.is<token::RightParen>()) {
            break;
        }

        auto expression = Expr::parse(parser);

        bool is_last_param = parser.next_token().visit(
            [](token::RightParen) -> bool { return true; },
            [](token::Comma) -> bool { return false; },
            [](auto&& arg) -> bool {
                using TokenType = std::decay_t<decltype(arg)>;
                static_assert(
                    std::is_base_of_v<token::Token<TokenType>, TokenType>);

                const token::Token<TokenType>& token = arg;

                throw error::UnexpectedToken(token.text(),
                                             token::RightParen::text(),
                                             token::Comma::text());
            });

        params.push_back(std::move(expression));

        if (is_last_param) {
            break;
        }
    }

    params.shrink_to_fit();

    const auto param_count = static_cast<uint8_t>(params.size());

    auto allocated_params = std::make_unique<Expr[]>(param_count);
    std::move(params.begin(), params.end(), allocated_params.get());

    return FunctionCall{Ident(identifier), std::move(allocated_params),
                        param_count};
}

auto node::Assignment::parse(Parser& parser, token::Identifier identifier)
    -> Assignment {
    parser.next_token().expect<token::Equals>();

    return Assignment{Ident(identifier),
                      std::make_unique<Expr>(Expr::parse(parser))};
}

auto node::Factor::parse(Parser& parser) -> Factor {
    const Token token = parser.peek_token();

    Factor factor = token.visit(
        [&parser](token::Identifier identifier) -> Factor {
            parser.next_token();

            return parser.peek_token().visit(
                [&parser, &identifier](token::LeftParen) -> Factor {
                    return std::make_unique<FunctionCall>(
                        FunctionCall::parse(parser, identifier));
                },
                [&parser, &identifier](token::Equals) -> Factor {
                    return Assignment::parse(parser, identifier);
                },
                [&identifier](auto&&) -> Factor { return Ident(identifier); });
        },
        [&parser](token::StringLiteral str) -> Factor {
            parser.next_token();

            return String{std::move(str.value)};
        },
        [&parser](token::CharacterLiteral character) -> Factor {
            parser.next_token();

            return Character{character.value};
        },
        [&parser](token::IntegerLiteral integer) -> Factor {
            parser.next_token();

            return Integer{integer.value};
        },
        [&parser](token::LeftParen) -> Factor {
            parser.next_token();

            auto expression = std::make_unique<Expr>(Expr::parse(parser));

            parser.next_token().expect<token::RightParen>();

            return Group{std::move(expression)};
        },
        [](auto&& arg) -> Factor {
            using TokenType = std::decay_t<decltype(arg)>;
            static_assert(
                std::is_base_of_v<token::Token<TokenType>, TokenType>);

            const token::Token<TokenType>& token = arg;

            throw error::UnexpectedToken(token.text(), token::Fn::text(),
                                         token::Var::text());
        });

    return factor;
}

auto node::Term::parse(Parser& parser) -> Term {
    auto factor = Factor::parse(parser);

    const Token token = parser.peek_token();

    if (token.is_not<token::Asterisk, token::ForwardSlash>()) {
        return factor;
    }

    const Token operation = parser.next_token();

    auto lhs = std::make_unique<Factor>(std::move(factor));
    auto rhs = std::make_unique<Term>(Term::parse(parser));

    Term term = operation.visit(
        [&lhs, &rhs](token::Asterisk) -> Term {
            return Mult{std::move(lhs), std::move(rhs)};
        },
        [&lhs, &rhs](token::ForwardSlash) -> Term {
            return Div{std::move(lhs), std::move(rhs)};
        },
        [](auto&& arg) -> Term {
            using TokenType = std::decay_t<decltype(arg)>;
            static_assert(
                std::is_base_of_v<token::Token<TokenType>, TokenType>);

            const token::Token<TokenType>& token = arg;

            throw error::UnexpectedToken(token.text(), token::Asterisk::text(),
                                         token::ForwardSlash::text());
        });

    return term;
}

auto node::Expr::parse(Parser& parser) -> Expr {
    auto term = Term::parse(parser);

    const Token token = parser.peek_token();

    if (token.is_not<token::Plus, token::Minus>()) {
        return term;
    }

    const Token operation = parser.next_token();

    auto lhs = std::make_unique<Term>(std::move(term));
    auto rhs = std::make_unique<Expr>(Expr::parse(parser));

    Expr expression = operation.visit(
        [&lhs, &rhs](token::Plus) -> Expr {
            return Add{std::move(lhs), std::move(rhs)};
        },
        [&lhs, &rhs](token::Minus) -> Expr {
            return Sub{std::move(lhs), std::move(rhs)};
        },
        [](auto&& arg) -> Expr {
            using TokenType = std::decay_t<decltype(arg)>;
            static_assert(
                std::is_base_of_v<token::Token<TokenType>, TokenType>);

            const token::Token<TokenType>& token = arg;

            throw error::UnexpectedToken(token.text(), token::Plus::text(),
                                         token::Minus::text());
        });

    return expression;
}

auto node::Block::parse(Parser& parser) -> Block {
    parser.next_token().expect<token::LeftBrace>();

    auto statements = std::vector<Stmt>();

    while (parser.peek_token().is_not<token::RightBrace, token::Eof>()) {
        auto statement = node::Stmt::parse(parser);

        statements.push_back(std::move(statement));
    }

    const size_t statement_count = statements.size();

    auto allocated_statements = std::make_unique<Stmt[]>(statement_count);
    std::move(statements.begin(), statements.end(), allocated_statements.get());

    return Block{std::move(allocated_statements), statement_count};
}

auto node::VarDecl::parse(Parser& parser) -> VarDecl {
    parser.next_token().expect<token::Var>();

    const auto ident = parser.next_token().get<token::Identifier>();
    const auto type = parser.next_token().get<token::Type>();

    parser.next_token().expect<token::Semicolon>();

    return VarDecl{Ident(ident), Type(type)};
}

auto node::FunctionDecl::parse(Parser& parser) -> FunctionDecl {
    parser.next_token().expect<token::Fn>();

    const auto ident = parser.next_token().get<token::Identifier>();

    parser.next_token().expect<token::LeftParen>();

    auto params = std::vector<Param>();

    while (true) {
        const Token token = parser.next_token();

        if (token.is<token::RightParen>()) {
            break;
        }

        const auto param_ident = token.get<token::Identifier>();
        const auto param_type = parser.next_token().get<token::Type>();

        bool is_last_param = parser.next_token().visit(
            [](token::RightParen) -> bool { return true; },
            [](token::Comma) -> bool { return false; },
            [](auto&& arg) -> bool {
                using TokenType = std::decay_t<decltype(arg)>;
                static_assert(
                    std::is_base_of_v<token::Token<TokenType>, TokenType>);

                const token::Token<TokenType>& token = arg;

                throw error::UnexpectedToken(token.text(),
                                             token::RightParen::text(),
                                             token::Comma::text());
            });

        params.emplace_back(Ident(param_ident), Type(param_type));

        if (params.size() > error::TooManyParameters::max_parameters()) {
            throw error::TooManyParameters(parser.pool->resolve(ident.index));
        }

        if (is_last_param) {
            break;
        }
    }

    params.shrink_to_fit();

    const auto return_type = parser.next_token().get<token::Type>();

    const auto param_count = static_cast<uint8_t>(params.size());

    auto allocated_params = std::make_unique<Param[]>(param_count);
    std::move(params.begin(), params.end(), allocated_params.get());

    return FunctionDecl{Ident(ident), std::move(allocated_params), param_count,
                        Type(return_type), Block::parse(parser)};
}

auto node::Return::parse(parser::Parser& parser) -> Return {
    parser.next_token().expect<token::Return>();

    auto expression = std::make_unique<Expr>(Expr::parse(parser));

    parser.next_token().expect<token::Semicolon>();

    return Return{std::move(expression)};
}

auto node::Stmt::parse(Parser& parser) -> Stmt {
    const Token token = parser.peek_token();

    Stmt statement = token.visit(
        [&parser](token::LeftBrace) -> Stmt { return Block::parse(parser); },
        [&parser](token::Var) -> Stmt { return VarDecl::parse(parser); },
        [&parser](token::Return) -> Stmt { return Return::parse(parser); },
        [&parser](auto&& arg) -> Stmt {
            try {
                auto expr = std::make_unique<Expr>(Expr::parse(parser));

                parser.next_token().expect<token::Semicolon>();

                return expr;
            } catch (...) {
                using TokenType = std::decay_t<decltype(arg)>;
                static_assert(
                    std::is_base_of_v<token::Token<TokenType>, TokenType>);

                const token::Token<TokenType>& token = arg;

                throw error::UnexpectedToken(token.text(), "statement");
            }
        });

    return statement;
}

auto node::Decl::parse(Parser& parser) -> Decl {
    const Token token = parser.peek_token();

    Decl decl = token.visit(
        [&parser](token::Var) -> Decl { return node::VarDecl::parse(parser); },
        [&parser](token::Fn) -> Decl {
            return std::make_unique<node::FunctionDecl>(
                node::FunctionDecl::parse(parser));
        },
        [](auto&& arg) -> Decl {
            using TokenType = std::decay_t<decltype(arg)>;
            static_assert(
                std::is_base_of_v<token::Token<TokenType>, TokenType>);

            const token::Token<TokenType>& token = arg;

            throw error::UnexpectedToken(token.text(), token::Fn::text(),
                                         token::Var::text());
        });

    return decl;
}

auto Parser::parse() && -> ast::Ast {
    std::vector<node::Decl> declarations;

    while (this->pos < this->tokens.size()) {
        declarations.push_back(node::Decl::parse(*this));
    }

    declarations.shrink_to_fit();

    const size_t node_count = declarations.size();

    auto nodes = std::make_unique<node::Decl[]>(node_count);
    std::move(declarations.begin(), declarations.end(), nodes.get());

    return {std::move(nodes), node_count};
}
