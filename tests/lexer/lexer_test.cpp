#include <string>

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators_all.hpp>
#include <catch2/matchers/catch_matchers_container_properties.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <tau/lexer/lexer.hpp>
#include <tau/lexer/token.hpp>
#include <tau/source/span.hpp>

using Catch::Matchers::Equals;
using Catch::Matchers::SizeIs;

using Catch::Matchers::StartsWith;

namespace lexer {
TEST_CASE("Lexer correctly tokenizes src", "[Lexer]") {
    SECTION("lex string literal") {
        Lexer lexer("\"test\"");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::StringLiteral>());
        CHECK(tokens[0].get<token::StringLiteral>().span == source::Span{0, 5});
        CHECK_THAT(std::string(lexer.pool()->resolve_string(
                       tokens[0].get<token::StringLiteral>().index)),
                   Equals("test"));
    }

    SECTION("lex string literal with escaped double quote") {
        Lexer lexer(R"("test\"")");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::StringLiteral>());
        CHECK(tokens[0].get<token::StringLiteral>().span == source::Span{0, 7});
        CHECK_THAT(std::string(lexer.pool()->resolve_string(
                       tokens[0].get<token::StringLiteral>().index)),
                   Equals(R"(test\")"));
    }

    SECTION("lex empty character literal") {
        Lexer lexer("''");

        REQUIRE_THROWS_WITH(lexer.lex(), StartsWith("empty character literal"));
    }

    SECTION("lex basic character literal") {
        Lexer lexer("'a'");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::CharacterLiteral>());
        CHECK(tokens[0].get<token::CharacterLiteral>().span ==
              source::Span{0, 2});
        CHECK(tokens[0].get<token::CharacterLiteral>().value == 'a');
    }

    SECTION("lex character literal with escaped single quote") {
        Lexer lexer("'\\''");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::CharacterLiteral>());
        CHECK(tokens[0].get<token::CharacterLiteral>().span ==
              source::Span{0, 3});
        CHECK(tokens[0].get<token::CharacterLiteral>().value == '\'');
    }

    SECTION("lex character literal with escaped newline") {
        Lexer lexer("'\\n'");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::CharacterLiteral>());
        CHECK(tokens[0].get<token::CharacterLiteral>().span ==
              source::Span{0, 3});
        CHECK(tokens[0].get<token::CharacterLiteral>().value == '\n');
    }

    SECTION("lex character literal with hex code") {
        Lexer lexer("'\\x2a'");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::CharacterLiteral>());
        CHECK(tokens[0].get<token::CharacterLiteral>().span ==
              source::Span{0, 5});
        CHECK(tokens[0].get<token::CharacterLiteral>().value == '\x2a');
    }

    SECTION("lex decimal integer literal") {
        Lexer lexer("358");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::IntegerLiteral>());
        CHECK(tokens[0].get<token::IntegerLiteral>().span ==
              source::Span{0, 2});
        CHECK(tokens[0].get<token::IntegerLiteral>().value == 358);
    }

    SECTION("lex binary integer literal") {
        Lexer lexer("0b100101");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::IntegerLiteral>());
        CHECK(tokens[0].get<token::IntegerLiteral>().span ==
              source::Span{0, 7});
        CHECK(tokens[0].get<token::IntegerLiteral>().value == 0b100101);
    }

    SECTION("lex octal integer literal") {
        Lexer lexer("0o641");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::IntegerLiteral>());
        CHECK(tokens[0].get<token::IntegerLiteral>().span ==
              source::Span{0, 4});
        CHECK(tokens[0].get<token::IntegerLiteral>().value == 0641);
    }

    SECTION("lex hex integer literal") {
        Lexer lexer("0x2b79");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::IntegerLiteral>());
        CHECK(tokens[0].get<token::IntegerLiteral>().span ==
              source::Span{0, 5});
        CHECK(tokens[0].get<token::IntegerLiteral>().value == 0x2b79);
    }

    SECTION("lex identifier") {
        Lexer lexer("testident");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::Identifier>());
        CHECK(tokens[0].get<token::Identifier>().span == source::Span{0, 8});
        CHECK_THAT(std::string(lexer.pool()->resolve_ident(
                       tokens[0].get<token::Identifier>().index)),
                   Equals("testident"));
    }

    SECTION("lex keyword") {
        Lexer lexer("func");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::Func>());
        CHECK(tokens[0].get<token::Func>().span == source::Span{0, 3});
    }

    SECTION("lex type") {
        Lexer lexer("Int");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::Type>());
        CHECK(tokens[0].get<token::Type>().span == source::Span{0, 2});
        CHECK_THAT(std::string(lexer.pool()->resolve_ident(
                       tokens[0].get<token::Type>().index)),
                   Equals("Int"));
    }

    SECTION("lex literal token") {
        Lexer lexer("/");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::ForwardSlash>());
        CHECK(tokens[0].get<token::ForwardSlash>().span == source::Span{0, 0});
    }

    SECTION("lex single equals") {
        Lexer lexer("=");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::Equals>());
        CHECK(tokens[0].get<token::Equals>().span == source::Span{0, 0});
    }

    SECTION("lexer skips whitespace") {
        Lexer lexer("    ");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::Eof>());
    }

    SECTION("lexer skips single-line comment") {
        Lexer lexer(R"(# this is first comment
                       # this is second comment)");

        TokenStream tokens;

        REQUIRE_NOTHROW(tokens = lexer.lex());
        REQUIRE_THAT(tokens, SizeIs(1));
        REQUIRE_NOTHROW(tokens[0].expect<token::Eof>());
    }
}
}  // namespace lexer
