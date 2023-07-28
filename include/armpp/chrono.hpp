#pragma once

#include <chrono>

namespace armpp::chrono {

using clock_rep = std::int32_t;

template <typename T = std::ratio<1>>
using duration = std::chrono::duration<clock_rep, T>;

template <typename T>
struct is_duration : std::false_type {};

template <typename Period>
struct is_duration<duration<Period>> : std::true_type {};

template <typename Period>
constexpr bool is_duration_v = is_duration<Period>::value;

template <typename Dur>
using enable_if_duration = std::enable_if_t<is_duration_v<Dur>, Dur>;

using picoseconds  = duration<std::pico>;
using nanoseconds  = duration<std::nano>;
using microseconds = duration<std::micro>;
using milliseconds = duration<std::milli>;
using seconds      = duration<>;
using minutes      = duration<std::ratio<60>>;
using hours        = duration<std::ratio<3600>>;

inline namespace literals {

inline constexpr picoseconds operator""_ps(unsigned long long v)
{
    return picoseconds{v};
}

inline constexpr nanoseconds operator""_ns(unsigned long long v)
{
    return nanoseconds{v};
}

inline constexpr microseconds operator""_us(unsigned long long v)
{
    return microseconds{v};
}

inline constexpr milliseconds operator""_ms(unsigned long long v)
{
    return milliseconds{v};
}

inline constexpr seconds operator""_s(unsigned long long v)
{
    return seconds{v};
}

}    // namespace literals

namespace static_tests {

static_assert(1000_ns == 1_us);

}    // namespace static_tests

}    // namespace armpp::chrono
