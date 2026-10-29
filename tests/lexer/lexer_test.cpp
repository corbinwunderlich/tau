#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>

#include <tau/lexer/lexer.hpp>

using lexer::Lexer;
namespace token = lexer::token;

TEST_CASE("Lexer correctly tokenizes src", "[Lexer]") {
    SECTION("lex string literal") {
        Lexer lexer("\"test\"");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::StringLiteral>(token).value == "test");
    }

    SECTION("lex string literal with escaped double quote") {
        Lexer lexer(R"("test\"")");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::StringLiteral>(token).value == "test\\\"");
    }

    SECTION("lex empty character literal") {
        Lexer lexer("''");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::Invalid>(token));
    }

    SECTION("lex basic character literal") {
        Lexer lexer("'a'");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::CharacterLiteral>(token).value == 'a');
    }

    SECTION("lex character literal with escaped single quote") {
        Lexer lexer("'\\''");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::CharacterLiteral>(token).value == '\'');
    }

    SECTION("lex character literal with escaped newline") {
        Lexer lexer("'\\n'");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::CharacterLiteral>(token).value == '\n');
    }

    SECTION("lex character literal with hex code") {
        Lexer lexer("'\\x2a'");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::CharacterLiteral>(token).value == '\x2a');
    }

    SECTION("lex decimal integer literal") {
        Lexer lexer("358");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::IntegerLiteral>(token).value == 358);
    }

    SECTION("lex binary integer literal") {
        Lexer lexer("0b100101");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::IntegerLiteral>(token).value == 0b100101);
    }

    SECTION("lex octal integer literal") {
        Lexer lexer("0o641");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::IntegerLiteral>(token).value == 0641);
    }

    SECTION("lex hex integer literal") {
        Lexer lexer("0x2b79");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::IntegerLiteral>(token).value == 0x2b79);
    }

    SECTION("lex identifier") {
        Lexer lexer("testident");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::Identifier>(token).name == "testident");
    }

    SECTION("lex keyword") {
        Lexer lexer("fn");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::Fn>(token));
    }

    SECTION("lex type") {
        Lexer lexer("Int");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::Type>(token).identifier == "Int");
    }

    SECTION("lex literal token") {
        Lexer lexer("+");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::Plus>(token));
    }

    SECTION("lex single equals") {
        Lexer lexer("=");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::Equals>(token));
    }

    SECTION("lex single-line comment") {
        Lexer lexer("// this is first comment\n// this is second comment");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Comment>(token).content == "// this is first comment");

        token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Comment>(token).content == "// this is second comment");
        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
    }

    SECTION("lex multi-line comment") {
        Lexer lexer("/* this is the first line\nand this is the second */");

        auto token = lexer.next_token();

        REQUIRE_NOTHROW(std::get<token::Eof>(lexer.next_token()));
        REQUIRE_NOTHROW(std::get<token::Comment>(token).content ==
                        "/* this is the first line\nand this is the second */");
    }
}
