#pragma once

#include <concepts>
#include <cstdint>

namespace armpp::util {

template <std::size_t Length, std::unsigned_integral T = std::uint32_t>
    requires(Length <= sizeof(T) * 8)
struct bit_sequence {
    static constexpr T value = (bit_sequence<Length - 1, T>::value << 1) | 1;
};

template <std::unsigned_integral T>
struct bit_sequence<1, T> {
    static constexpr T value = 1;
};

template <std::size_t Length, std::unsigned_integral T = std::uint32_t>
constexpr T bit_sequence_v = bit_sequence<Length, T>::value;

static_assert(bit_sequence_v<1> == 0b1);
static_assert(bit_sequence_v<3> == 0b111);

template <std::size_t Offset, std::size_t Length, std::unsigned_integral T = std::uint32_t>
    requires(Length + Offset <= sizeof(T) * 8)
struct bit_mask {
    static constexpr T value = bit_sequence_v<Length, T> << Offset;
};

template <std::size_t Offset, std::size_t Length, std::unsigned_integral T = std::uint32_t>
constexpr T bit_mask_v = bit_mask<Offset, Length, T>::value;

static_assert(bit_mask_v<0, 2> == 0b11);
static_assert(bit_mask_v<3, 5> == 0b11111000);

}    // namespace armpp::util
