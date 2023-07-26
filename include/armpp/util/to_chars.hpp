#pragma once

#include <concepts>
#include <cstdint>

namespace armpp::util {

inline void
reverse_string(char* first, char* last)
{
    for (; first < last; ++first, --last) {
        auto tmp = *first;
        *first   = *last;
        *last    = tmp;
    }
}

enum class number_base { bin = 2, oct = 8, dec = 10, hex = 16 };

template <std::integral Integer>
void
to_chars(char* buffer, std::size_t buffer_length, Integer value,
         number_base base = number_base::dec, std::int8_t width = 0, char fill = ' ')
{
    constexpr auto byte_count = sizeof(Integer);
    constexpr auto bit_count  = byte_count * 8;
    constexpr char digit_chars[]
        = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

    // TODO check buffer size
    if (base == number_base::bin) {
        if (width <= 0)
            width = bit_count;
        while (width > 0) {
            switch ((value >> (width - 1) & 1)) {
            case 0:
                *buffer++ = '0';
                break;
            case 1:
                *buffer++ = '1';
                break;
            }
            --width;
            if (width % 8 == 0 && width != 0) {
                *buffer++ = ' ';
            }
        }
        *buffer++ = 0;
    } else {
        auto sign = false;
        if constexpr (std::is_signed_v<Integer>) {
            if (base == number_base::dec) {
                sign = value & (1 << (bit_count - 1));
                if (sign)
                    value = -value;
            } else {
                to_chars(buffer, buffer_length, static_cast<std::make_unsigned_t<Integer>>(value),
                         base, width, fill);
                return;
            }
        }
        auto current = buffer;
        if (value == 0) {
            *current++ = '0';
        } else {
            auto base_value = static_cast<std::underlying_type_t<number_base>>(base);
            while (value > 0) {
                *current++ = digit_chars[value % base_value];
                value /= base_value;
            }
        }
        if (sign) {
            *current++ = '-';
        }
        for (; current < buffer + buffer_length && current <= buffer + width;) {
            *current++ = fill;
        }
        --current;
        reverse_string(buffer, current);
        ++current;
        *current = 0;
    }
}

template <typename T>
void
to_chars(char* buffer, std::size_t buffer_length, T* pointer)
{
    to_chars(buffer, buffer_length, reinterpret_cast<std::ptrdiff_t>(pointer), number_base::hex,
             sizeof(std::ptrdiff_t) * 2, '0');
}

}    // namespace armpp::util
