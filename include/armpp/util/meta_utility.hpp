#pragma once

#include <type_traits>

namespace armpp::util {

template <typename... T>
struct first_type;

template <typename T, typename... U>
struct first_type<T, U...> {
    using type = T;
};

template <typename... T>
using first_type_t = typename first_type<T...>::type;

template <typename T, typename... U>
constexpr bool are_same_v = (std::is_same_v<T, U> && ...);

template <typename T>
constexpr bool are_same_v<T> = true;

}    // namespace armpp::util
