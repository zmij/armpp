#pragma once

#include <armpp/hal/registers.hpp>

#include <cstdint>

namespace armpp::hal::uart {

enum class number_base { bin = 2, oct = 8, dec = 10, hex = 16 };

using data_register = read_write_register<raw_register, 0, 8>;
static_assert(sizeof(data_register) == 4);

union state_register {
    union {
        bool_read_only_register<0> tx_buffer_full;
        bool_read_only_register<1> rx_buffer_full;
        bit_read_write_register<2> tx_buffer_overrun;
        bit_read_write_register<3> rx_buffer_overrun;
    };
    raw_register volatile raw;
};
static_assert(sizeof(state_register) == 4);

template <register_mode Mode = register_mode::volatile_reg>
union control_register {
    union {
        bool_read_write_register<0, Mode> tx_enable;
        bool_read_write_register<1, Mode> rx_enable;
        bool_read_write_register<2, Mode> tx_interrupt_enable;
        bool_read_write_register<3, Mode> rx_interrupt_enable;
        bool_read_write_register<4, Mode> tx_overrun_interrupt_enable;
        bool_read_write_register<5, Mode> rx_overrun_interrupt_enable;
        bool_read_write_register<6, Mode> hs_test_mode;
    };
    raw_register volatile raw;

    constexpr control_register() : raw{0} {}
};
static_assert(sizeof(control_register<>) == 4);

union interrupt_register {
    union {
        bool_read_only_register<0> tx_interrupt;
        bool_read_only_register<1> rx_interrupt;
        bool_read_only_register<2> tx_overrun_interrupt;
        bool_read_only_register<3> rx_overrun_interrupt;

        bit_write_only_register<0> tx_interrupt_reset;
        bit_write_only_register<1> rx_interrupt_reset;
        bit_write_only_register<2> tx_overrun_interrupt_reset;
        bit_write_only_register<3> rx_overrun_interrupt_reset;
    };
    raw_register volatile raw;
};
static_assert(sizeof(interrupt_register) == 4);

using bauddiv_register = raw_register;    // read_write_register<raw_register, 0, 20>;
static_assert(sizeof(bauddiv_register) == 4);

// TODO Maybe flags?
struct uart_init {
    struct tx_rx {
        bool tx;
        bool rx;
    };

    tx_rx         enable;
    tx_rx         enable_interrupt;
    tx_rx         enable_overrun_interrupt;
    std::uint32_t baud_rate;
    bool          enable_hs_test_mode;
};

constexpr std::uint8_t
digits_per_byte(number_base base)
{
    switch (base) {
    case number_base::bin:
        return 8;
    case number_base::oct:
        return 4;
    case number_base::dec:
        return 3;
    case number_base::hex:
        return 2;
    }
}

namespace detail {

inline void
reverse_string(char* first, char* last)
{
    for (; first < last; ++first, --last) {
        auto tmp = *first;
        *first   = *last;
        *last    = tmp;
    }
}

}    // namespace detail

//----------------------------------------------------------------------------
/**
 * @brief
 *
 */
class uart {
public:
    bool
    tx_buffer_full() const
    {
        return state_.tx_buffer_full;
    }
    bool
    rx_buffer_full() const
    {
        return state_.rx_buffer_full;
    }
    bool
    tx_buffer_overrun() const
    {
        return state_.tx_buffer_overrun;
    }
    bool
    rx_buffer_overrun() const
    {
        return state_.rx_buffer_overrun;
    }
    void
    reset_tx_buffer_overrun()
    {
        state_.tx_buffer_overrun = 1;
    }
    void
    reset_rx_buffer_overrun()
    {
        state_.rx_buffer_overrun = 1;
    }

    void
    put(char c)    // TODO timeout and error status
    {
        while (tx_buffer_full())
            ;
        data_ = static_cast<raw_register>(c);
    }

    void
    write(char const* str)
    {
        for (; *str != 0; ++str) {
            put(*str);
        }
    }

    template <std::integral Integer>
    void
    write(Integer val, number_base base, std::int8_t width, char fill = ' ')
    {
        constexpr auto byte_count = sizeof(Integer);
        constexpr auto bit_count  = byte_count * 8;
        constexpr char digit_chars[]
            = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

        if (width == 0 && base == number_base::bin) {
            width = byte_count * 8;
        }
        if (base == number_base::bin) {
            // For binary numbers output requested bit count
            while (width > 0) {
                switch ((val >> (width - 1) & 1)) {
                case 0:
                    put('0');
                    break;
                case 1:
                    put('1');
                    break;
                }
                --width;
                if (width % 8 == 0 && width != 0) {
                    put(' ');
                }
            }
        } else {
            auto sign = false;
            if constexpr (std::is_signed_v<Integer>) {
                if (base == number_base::dec) {
                    sign = val & (1 << (bit_count - 1));
                    if (sign) {
                        val = -val;
                    }
                } else {
                    write(static_cast<std::make_unsigned_t<Integer>>(val), base, width);
                    return;
                }
            }

            std::uint8_t digit_count = 0U;
            char         buff[byte_count * 4];
            if (val == 0) {
                *buff       = '0';
                digit_count = 1;
            } else {
                auto base_value = static_cast<std::underlying_type_t<number_base>>(base);
                while (val > 0) {
                    buff[digit_count] = digit_chars[val % base_value];
                    val /= base_value;
                    ++digit_count;
                }
            }
            if (sign) {
                buff[digit_count] = '-';
                ++digit_count;
            }
            for (; digit_count < width && digit_count < byte_count * 4; ++digit_count) {
                buff[digit_count] = fill;
            }
            detail::reverse_string(buff, buff + (digit_count - 1));
            buff[digit_count] = 0;
            write(buff);
        }
    }

    char
    get()    // TODO timeout and error status
    {
        while (!tx_buffer_full())
            ;
        return static_cast<char>(data_.get());
    }

private:
    friend class uart_handle;

    void    // TODO Error status
    configure(uart_init const& init);

private:
    data_register      data_;      /*<! Data value */
    state_register     state_;     /*<! State register */
    control_register<> ctrl_;      /*<! Control register */
    interrupt_register interrupt_; /*<! Interrupt status/clear register */
    bauddiv_register   bauddiv_;   /*<! Baud rate divider register, min value 16 */
};

static_assert(sizeof(uart) == 4 * 5);

//----------------------------------------------------------------------------
/**
 * @brief
 *
 */
class uart_handle {
public:
    uart_handle(address device_address, uart_init const& init) noexcept
        : device_(*reinterpret_cast<uart*>(device_address))
    {
        // TODO check for error status and report it somehow
        device_.configure(init);
    }

    uart&
    operator*() noexcept
    {
        return device_;
    }

    uart const&
    operator*() const noexcept
    {
        return device_;
    }

    uart*
    operator->() noexcept
    {
        return &device_;
    }

    uart const*
    operator->() const noexcept
    {
        return &device_;
    }

    void
    set_output_number_base(number_base val)
    {
        output_number_base_ = val;
    }
    number_base
    get_output_number_base() const
    {
        return output_number_base_;
    }

    void
    set_output_width(std::uint8_t val)
    {
        output_width_ = val;
    }
    std::uint8_t
    get_output_width() const
    {
        return output_width_;
    }

private:
    uart& device_;

    number_base  output_number_base_ = number_base::bin;
    std::uint8_t output_width_       = 0;
};

//----------------------------------------------------------------------------
// Output
inline uart_handle&
operator<<(uart_handle& dev, char c)
{
    dev->put(c);
    return dev;
}

inline uart_handle&
operator<<(uart_handle& dev, char const* str)
{
    dev->write(str);
    return dev;
}

template <std::integral T>
uart_handle&
operator<<(uart_handle& dev, T val)
{
    dev->write(val, dev.get_output_number_base(), dev.get_output_width());
    return dev;
}

template <readable_field F>
uart_handle&
operator<<(uart_handle& dev, F const& reg)
{
    dev->write(reg.get(), dev.get_output_number_base(), dev.get_output_width());
    return dev;
}

inline uart_handle&
operator<<(uart_handle& dev, void (*fp)(uart_handle&))
{
    fp(dev);
    return dev;
}

//----------------------------------------------------------------------------
// Manipulators
namespace detail {

struct width_setter {

    std::uint8_t target_width;

    friend uart_handle&
    operator<<(uart_handle& dev, width_setter const& wset)
    {
        dev.set_output_width(wset.target_width);
        return dev;
    }
};

}    // namespace detail
inline void
bin_out(uart_handle& dev)
{
    dev.set_output_number_base(number_base::bin);
}

inline void
oct_out(uart_handle& dev)
{
    dev.set_output_number_base(number_base::oct);
}

inline void
dec_out(uart_handle& dev)
{
    dev.set_output_number_base(number_base::dec);
}

inline void
hex_out(uart_handle& dev)
{
    dev.set_output_number_base(number_base::hex);
}

inline detail::width_setter
width_out(std::uint8_t w)
{
    return {w};
}

}    // namespace armpp::hal::uart
