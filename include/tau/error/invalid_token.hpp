#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace error {
class InvalidToken : public std::runtime_error {
  private:
    static auto quote(std::string_view sv) -> std::string {
        return std::string().append(sv);
    }

    static auto quote(char c) -> std::string {
        return std::string{"`"} + c + '`';
    }

    template <typename Found>
    auto build_message(Found found) -> std::string {
        std::string message = "expected valid token, found " + quote(found);

        return message;
    }

  public:
    template <typename Found>
    InvalidToken(Found found) : std::runtime_error(build_message(found)) {}
};
}  // namespace error
