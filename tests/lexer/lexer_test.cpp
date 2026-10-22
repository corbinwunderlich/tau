#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include "tau/lexer/lexer.hpp"

TEST_CASE("Lexer correctly tokenizes src", "[Lexer]") {
    SECTION("lex string literal") {
        lexer::Lexer lexer("\"test\"");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::StringLiteral>(token).value == "test");
    }

    SECTION("lex string literal with escaped double quote") {
        lexer::Lexer lexer("\"test\\\"\"");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::StringLiteral>(token).value == "test\\\"");
    }

    SECTION("lex empty character literal") {
        lexer::Lexer lexer("''");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::Invalid>(token));
    }

    SECTION("lex basic character literal") {
        lexer::Lexer lexer("'a'");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::CharacterLiteral>(token).value == 'a');
    }

    SECTION("lex character literal with escaped single quote") {
        lexer::Lexer lexer("'\\''");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::CharacterLiteral>(token).value == '\'');
    }

    SECTION("lex character literal with escaped newline") {
        lexer::Lexer lexer("'\\n'");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::CharacterLiteral>(token).value == '\n');
    }

    SECTION("lex character literal with hex code") {
        lexer::Lexer lexer("'\\x2a'");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::CharacterLiteral>(token).value == '\x2a');
    }

    SECTION("lex decimal integer literal") {
        lexer::Lexer lexer("358");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::IntegerLiteral>(token).value == 358);
    }

    SECTION("lex binary integer literal") {
        lexer::Lexer lexer("0b100101");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::IntegerLiteral>(token).value == 0b100101);
    }

    SECTION("lex octal integer literal") {
        lexer::Lexer lexer("0o641");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::IntegerLiteral>(token).value == 0641);
    }

    SECTION("lex hex integer literal") {
        lexer::Lexer lexer("0x2b79");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::IntegerLiteral>(token).value == 0x2b79);
    }

    SECTION("lex identifier") {
        lexer::Lexer lexer("testident");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::Identifier>(token).name == "testident");
    }

    SECTION("lex keyword") {
        lexer::Lexer lexer("if");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::If>(token));
    }

    SECTION("lex literal token") {
        lexer::Lexer lexer("+");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::Plus>(token));
    }

    SECTION("lex single equals") {
        lexer::Lexer lexer("=");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::Equals>(token));
    }

    SECTION("lex double equals") {
        lexer::Lexer lexer("==");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::DoubleEquals>(token));
    }

    SECTION("lex single-line comment") {
        lexer::Lexer lexer("// this is first comment\n// this is second comment");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Comment>(token).content ==
                        "// this is first comment");

        token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Comment>(token).content ==
                        "// this is second comment");
        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
    }

    SECTION("lex multi-line comment") {
        lexer::Lexer lexer("/* this is the first line\nand this is the second */");

        auto token = lexer.nextToken();

        REQUIRE_NOTHROW(std::get<lexer::token::Eof>(lexer.nextToken()));
        REQUIRE_NOTHROW(std::get<lexer::token::Comment>(token).content ==
                        "/* this is the first line\nand this is the second */");
    }
}
