#include <stdexcept>
#include <string>
#include <string_view>

#include <tau/error/too_many_parameters.hpp>

using error::TooManyParameters;

namespace {
auto build_too_many_parameters(std::string_view identifier) -> std::string {
    std::string message = "function ";

    message.append(identifier)
        .append(" has too many parameters: can only have ")
        .append(std::to_string(TooManyParameters::max_parameters()));

    return message;
}
}  // namespace

TooManyParameters::TooManyParameters(std::string_view identifier)
    : std::runtime_error(build_too_many_parameters(identifier)) {}
