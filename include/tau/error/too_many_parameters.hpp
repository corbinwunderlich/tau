#pragma once

#include <cstdint>
#include <limits>
#include <stdexcept>

namespace error {
class TooManyParameters : public std::runtime_error {
  public:
    TooManyParameters(std::string_view identifier);

    static constexpr auto max_parameters() -> uint8_t {
        return std::numeric_limits<uint8_t>::max();
    }
};
}  // namespace error
