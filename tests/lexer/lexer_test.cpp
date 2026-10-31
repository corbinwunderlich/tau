#include <string>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <tau/lexer/lexer.hpp>
#include <tau/lexer/token.hpp>
#include <tau/src/src.hpp>

using Catch::Matchers::Equals;
using Catch::Matchers::SizeIs;

using lexer::Lexer;
using lexer::Token;
namespace token = lexer::token;

TEST_CASE("Lexer correctly tokenizes src", "[Lexer]") {
    SECTION("lex string literal") {
        Lexer lexer("\"test\"");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::StringLiteral>());
        CHECK(tokens[0].get<token::StringLiteral>().value == "test");
    }

    SECTION("lex string literal with escaped double quote") {
        Lexer lexer(R"("test\"")");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::StringLiteral>());
        CHECK(tokens[0].get<token::StringLiteral>().value == R"(test\")");
    }

    SECTION("lex empty character literal") {
        Lexer lexer("''");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::Invalid>());
    }

    SECTION("lex basic character literal") {
        Lexer lexer("'a'");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::CharacterLiteral>());
        CHECK(tokens[0].get<token::CharacterLiteral>().value == 'a');
    }

    SECTION("lex character literal with escaped single quote") {
        Lexer lexer("'\\''");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::CharacterLiteral>());
        CHECK(tokens[0].get<token::CharacterLiteral>().value == '\'');
    }

    SECTION("lex character literal with escaped newline") {
        Lexer lexer("'\\n'");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::CharacterLiteral>());
        CHECK(tokens[0].get<token::CharacterLiteral>().value == '\n');
    }

    SECTION("lex character literal with hex code") {
        Lexer lexer("'\\x2a'");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::CharacterLiteral>());
        CHECK(tokens[0].get<token::CharacterLiteral>().value == '\x2a');
    }

    SECTION("lex decimal integer literal") {
        Lexer lexer("358");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::IntegerLiteral>());
        CHECK(tokens[0].get<token::IntegerLiteral>().value == 358);
    }

    SECTION("lex binary integer literal") {
        Lexer lexer("0b100101");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::IntegerLiteral>());
        CHECK(tokens[0].get<token::IntegerLiteral>().value == 0b100101);
    }

    SECTION("lex octal integer literal") {
        Lexer lexer("0o641");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::IntegerLiteral>());
        CHECK(tokens[0].get<token::IntegerLiteral>().value == 0641);
    }

    SECTION("lex hex integer literal") {
        Lexer lexer("0x2b79");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::IntegerLiteral>());
        CHECK(tokens[0].get<token::IntegerLiteral>().value == 0x2b79);
    }

    SECTION("lex identifier") {
        Lexer lexer("testident");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::Identifier>());
        CHECK_THAT(std::string(lexer.extract_pool()->resolve(
                       tokens[0].get<token::Identifier>().index)),
                   Equals("testident"));
    }

    SECTION("lex keyword") {
        Lexer lexer("fn");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::Fn>());
    }

    SECTION("lex type") {
        Lexer lexer("Int");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::Type>());
        CHECK_THAT(std::string(lexer.extract_pool()->resolve(
                       tokens[0].get<token::Type>().index)),
                   Equals("Int"));
    }

    SECTION("lex literal token") {
        Lexer lexer("+");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::Plus>());
    }

    SECTION("lex single equals") {
        Lexer lexer("=");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::Equals>());
    }

    SECTION("lex single-line comment") {
        Lexer lexer("// this is first comment\n// this is second comment");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(2));

        REQUIRE_NOTHROW(tokens[0].expect<token::Comment>());
        CHECK_THAT(std::string(tokens[0].get<token::Comment>().content),
                   Equals("// this is first comment"));

        REQUIRE_NOTHROW(tokens[1].expect<token::Comment>());
        CHECK_THAT(std::string(tokens[1].get<token::Comment>().content),
                   Equals("// this is second comment"));
    }

    SECTION("lex multi-line comment") {
        Lexer lexer("/* this is the first line\nand this is the second */");

        auto tokens = lexer.lex();

        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::Comment>());
        CHECK_THAT(
            std::string(tokens[0].get<token::Comment>().content),
            Equals("/* this is the first line\nand this is the second */"));
    }
}
