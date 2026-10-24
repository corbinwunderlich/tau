#pragma once

#include <algorithm>
#include <array>
#include <optional>
#include <string_view>

#include "token.hpp"

namespace lexer {
class Keywords {
  private:
    struct Keyword {
        std::string_view text;
        Token token;
    };

    static constexpr inline auto lookupTable = []() {
        std::array keywords{
            Keyword{"fn", lexer::token::Fn{}},
            Keyword{"return", lexer::token::Return{}},

            Keyword{"let", lexer::token::Let{}},
            Keyword{"var", lexer::token::Var{}},

            Keyword{"if", lexer::token::If{}},
            Keyword{"else", lexer::token::Else{}},

            Keyword{"while", lexer::token::While{}},
            Keyword{"for", lexer::token::For{}},
            Keyword{"continue", lexer::token::Continue{}},
            Keyword{"break", lexer::token::Break{}},

            Keyword{"str", lexer::token::Str{}},
            Keyword{"char", lexer::token::Char{}},

            Keyword{"int8", lexer::token::Int8{}},
            Keyword{"uint8", lexer::token::Uint8{}},
            Keyword{"int16", lexer::token::Int16{}},
            Keyword{"uint16", lexer::token::Uint16{}},
            Keyword{"int32", lexer::token::Int32{}},
            Keyword{"uint32", lexer::token::Uint32{}},
            Keyword{"int64", lexer::token::Int64{}},
            Keyword{"uint64", lexer::token::Uint64{}},
            Keyword{"usize", lexer::token::Usize{}},
        };

        if constexpr (keywords.size() < 2) {
            return keywords;
        }

        for (size_t i = 0; i < keywords.size() - 1; i++) {
            for (size_t k = 0; k < keywords.size() - i - 1; k++) {
                if (keywords[k].text < keywords[k + 1].text) {
                    continue;
                }

                Keyword swap = keywords[k];

                keywords[k] = keywords[k + 1];
                keywords[k + 1] = swap;
            }
        }

        return keywords;
    }();

  public:
    [[nodiscard]] static constexpr std::optional<Token>
    lookup(std::string_view identifier) noexcept {
        const Keyword *keyword = std::lower_bound(
            lookupTable.begin(), lookupTable.end(), identifier,
            [](const struct Keyword &keyword, std::string_view text) {
                return keyword.text < text;
            });

        if (keyword == lookupTable.end() || keyword->text != identifier) {
            return std::nullopt;
        }

        return keyword->token;
    }
};
} // namespace lexer
