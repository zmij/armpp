#pragma once

#include <concepts>

namespace armpp::concepts {

template <typename T>
concept enumeration = std::is_enum_v<T>;

}    // namespace armpp::concepts
