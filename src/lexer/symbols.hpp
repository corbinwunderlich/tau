#pragma once

#include <array>
#include <string_view>

#include <tau/lexer/token.hpp>
#include <tau/source/span.hpp>

namespace lexer::detail {
struct SymbolEntry {
    std::string_view lexeme;
    auto (*make)(source::Span) -> Token;
};

#define TAU_SYMBOL_ENTRY(Name, Lexeme, ...) \
    SymbolEntry{Lexeme,                     \
                [](source::Span span) -> Token { return token::Name{span}; }},

inline constexpr std::array symbols = {TAU_SYMBOLS(TAU_SYMBOL_ENTRY)};

#undef TAU_SYMBOL_ENTRY
}  // namespace lexer::detail
