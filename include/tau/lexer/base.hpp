#pragma once

#include <cstdint>

namespace lexer {
enum class Base : uint8_t { Binary = 2, Octal = 8, Decimal = 10, Hexadecimal = 16 };
}
