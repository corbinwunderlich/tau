#pragma once

#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace parser::error {
class UnexpectedToken : public std::runtime_error {
  private:
    static auto quote(std::string_view sv) -> std::string { return std::string().append(sv); }
    static auto quote(char c) -> std::string { return std::string{"`"} + c + '`'; }

    template <typename Found, typename Expected>
    static auto build_message(Found &&found, Expected &&expected) -> std::string {
        std::string message = "expected " + quote(expected) + ',' + " found " + quote(found);

        return message;
    }

    template <typename Found, typename... Expected>
    static auto build_message(Found &&found, Expected &&...expected) -> std::string {
        std::string message = "expected one of ";

        ((message.append(quote(expected)).append(1, ',')), ...);

        message.append(" found ").append(quote(found));

        return message;
    }

  public:
    template <typename Found, typename Expected>
    UnexpectedToken(Found found, Expected &&expected)
        : std::runtime_error(this->build_message(found, expected)) {};

    template <typename Found, typename... Expected>
    UnexpectedToken(Found found, Expected &&...expected)
        : std::runtime_error(this->build_message(found, std::forward<Expected>(expected)...)) {};
};

class TooManyParameters : public std::runtime_error {
  private:
    static auto build_message(std::string_view identifier) -> std::string {
        std::string message = "function ";

        message.append(identifier)
            .append(" has too many parameters: can only have ")
            .append(std::to_string(max_parameters()));

        return message;
    }

  public:
    TooManyParameters(std::string_view identifier)
        : std::runtime_error(build_message(identifier)) {};

    static constexpr auto max_parameters() -> uint8_t {
        return std::numeric_limits<uint8_t>::max();
    }
};
} // namespace parser::error
