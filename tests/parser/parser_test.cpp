#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_predicate.hpp>
#include <catch2/matchers/catch_matchers_quantifiers.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <tau/ast/ast.hpp>
#include <tau/ast/node.hpp>
#include <tau/lexer/lexer.hpp>
#include <tau/parser/parser.hpp>

using Catch::Matchers::AllMatch;
using Catch::Matchers::Equals;
using Catch::Matchers::SizeIs;

using lexer::Lexer;
using parser::Parser;
namespace node = ast::node;

TEST_CASE("Parser correctly constructs AST", "[Parser]") {
    SECTION("parse var declaration") {
        Lexer lexer{"var x Int;"};
        Parser parser{lexer};

        ast::Ast ast;

        REQUIRE_NOTHROW(ast = parser.parse());
        REQUIRE_THAT(ast.nodes, SizeIs(1));

        node::VarDecl decl;

        REQUIRE_NOTHROW(decl = std::get<node::VarDecl>(ast.nodes[0]));
        REQUIRE_THAT(std::string(decl.ident.name(*lexer.pool())), Equals("x"));
    }

    SECTION("parse function declaration") {
        Lexer lexer{"fn add(first First, second Second) Return { }"};
        Parser parser{lexer};

        ast::Ast ast;

        REQUIRE_NOTHROW(ast = parser.parse());
        REQUIRE_THAT(ast.nodes, SizeIs(1));

        node::FunctionDecl decl;

        REQUIRE_NOTHROW(
            decl = std::get<node::FunctionDecl>(std::move(ast.nodes[0])));
        CHECK_THAT(std::string(decl.ident.name(*lexer.pool())), Equals("add"));
        REQUIRE_THAT(std::string(decl.return_type.name(*lexer.pool())),
                     Equals("Return"));
        REQUIRE_THAT(decl.params, SizeIs(2));
        CHECK_THAT(std::string(decl.params[0].ident.name(*lexer.pool())),
                   Equals("first"));
        CHECK_THAT(std::string(decl.params[0].type.name(*lexer.pool())),
                   Equals("First"));
        CHECK_THAT(std::string(decl.params[1].ident.name(*lexer.pool())),
                   Equals("second"));
        CHECK_THAT(std::string(decl.params[1].type.name(*lexer.pool())),
                   Equals("Second"));
    }
}
