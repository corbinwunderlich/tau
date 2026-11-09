#include <string>

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
        REQUIRE_THAT(
            ast.dump(*lexer.pool()).str(),
            Equals(
                R"([{"var_decl":{"ident":{"name":"x"},"type":{"name":"Int"}}}])"));
    }

    SECTION("parse function declaration without parameters") {
        Lexer lexer{"fn main() Void { }"};
        Parser parser{lexer};

        ast::Ast ast;

        REQUIRE_NOTHROW(ast = parser.parse());
        REQUIRE_THAT(
            ast.dump(*lexer.pool()).str(),
            Equals(
                R"([{"function_decl":{"ident":{"name":"main"},"params":[],"return_type":{"name":"Void"},"body":{"statements":[]}}}])"));
    }

    SECTION("parse function declaration with parameters") {
        Lexer lexer{"fn add(first First, second Second) Return { }"};
        Parser parser{lexer};

        ast::Ast ast;

        REQUIRE_NOTHROW(ast = parser.parse());
        REQUIRE_THAT(
            ast.dump(*lexer.pool()).str(),
            Equals(
                R"([{"function_decl":{"ident":{"name":"add"},"params":[{"ident":{"name":"first"},"type":{"name":"First"}},{"ident":{"name":"second"},"type":{"name":"Second"}}],"return_type":{"name":"Return"},"body":{"statements":[]}}}])"));
    }
}
