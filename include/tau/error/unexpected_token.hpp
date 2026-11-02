#pragma once

#include <stdexcept>

namespace error {
class UnexpectedToken : public std::runtime_error {
  private:
    static auto quote(std::string_view sv) -> std::string {
        return std::string().append(sv);
    }

    static auto quote(char c) -> std::string {
        return std::string{"`"} + c + '`';
    }

    template <typename Found, typename Expected>
    static auto build(Found&& found, Expected&& expected) -> std::string {
        std::string message =
            "expected " + quote(expected) + ',' + " found " + quote(found);

        return message;
    }

    template <typename Found, typename... Expected>
    static auto build(Found&& found, Expected&&... expected) -> std::string {
        std::string message = "expected one of ";

        ((message.append(quote(expected)).append(1, ',')), ...);

        message.append(" found ").append(quote(found));

        return message;
    }

  public:
    template <typename Found, typename Expected>
    UnexpectedToken(Found found, Expected&& expected)
        : std::runtime_error(build(found, expected)) {}

    template <typename Found, typename... Expected>
    UnexpectedToken(Found found, Expected&&... expected)
        : std::runtime_error(
              build(found, std::forward<Expected>(expected)...)) {}
};
}  // namespace error
