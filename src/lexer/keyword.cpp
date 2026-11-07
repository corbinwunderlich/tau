#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <type_traits>
#include <utility>

#include <tau/lexer/keyword.hpp>
#include <tau/lexer/token.hpp>

namespace lexer::keyword {
namespace {
template <typename T, typename = void>
struct IsIter : std::false_type {};

template <typename T>
struct IsIter<T,
              std::void_t<decltype(std::declval<T>().begin()),
                          decltype(std::declval<T>().end())>> : std::true_type {
};

constexpr uint32_t fnv_a1_base = 0x811c9dc5;
constexpr uint32_t fnv_a1_prime = 0x01000193;

template <typename T, typename = std::enable_if_t<IsIter<T>::value>>
constexpr auto hash(T identifier) -> uint32_t {
    uint32_t hash = fnv_a1_base;

    for (const char byte : identifier) {
        hash ^= static_cast<uint32_t>(byte);
        hash *= fnv_a1_prime;
    }

    return hash;
}

template <size_t N>
constexpr auto hash(const char (&identifier)[N]) -> uint32_t {
    std::array<char, N - 1> data{};

    for (size_t i = 0; i < N - 1; i++) {
        data[i] = identifier[i];
    }

    return hash(data);
}

template <size_t N>
constexpr auto matches(std::string_view identifier, const char (&keyword)[N])
    -> bool {
    if (identifier.size() != N - 1) {
        return false;
    }

    for (size_t i = 0; i < N - 1; i++) {
        if (identifier[i] == keyword[i]) {
            continue;
        }

        return false;
    }

    return true;
}
}  // namespace

auto lookup(std::string_view identifier) -> std::optional<Token> {
    const uint32_t identifier_hash = hash(identifier);

    switch (identifier_hash) {
#define TAU_KEYWORDS_AS_SWITCH_CASE(type, lexeme, ...) \
    case hash(lexeme): {                               \
        if (matches(identifier, lexeme)) {             \
            return lexer::token::type{};               \
        }                                              \
        break;                                         \
    }

        TAU_KEYWORDS(TAU_KEYWORDS_AS_SWITCH_CASE)
#undef TAU_KEYWORDS_AS_SWITCH_CASE
        default:
            break;
    }

    return std::nullopt;
}
}  // namespace lexer::keyword
