#pragma once

#include <armpp/hal/registers.hpp>
#include <armpp/util/to_chars.hpp>

#include <cstdint>

/**
 * @namespace armpp::hal::uart
 * @brief Namespace containing UART related classes and structures
 */
namespace armpp::hal::uart {

using number_base = util::number_base;

/**
 * @typedef UART data register
 */
using data_register = read_write_register_field<raw_register, 0, 8>;
static_assert(sizeof(data_register) == 4);

/**
 * @typedef state_register
 * @brief Union for manipulating state register fields
 */
union state_register {
    bool_read_only_register_field<0> tx_buffer_full;
    bool_read_only_register_field<1> rx_buffer_full;
    bit_read_write_register_field<2> tx_buffer_overrun;
    bit_read_write_register_field<3> rx_buffer_overrun;

    raw_register volatile raw;
};
static_assert(sizeof(state_register) == 4);

/**
 * @typedef control_register
 * @brief Union for manipulating control register fields
 * @tparam Mode Register mode (volatile/non volatile)
 */
template <register_mode Mode = register_mode::volatile_reg>
union control_register {
    bool_read_write_register_field<0, access_mode::field, Mode> tx_enable;
    bool_read_write_register_field<1, access_mode::field, Mode> rx_enable;
    bool_read_write_register_field<2, access_mode::field, Mode> tx_interrupt_enable;
    bool_read_write_register_field<3, access_mode::field, Mode> rx_interrupt_enable;
    bool_read_write_register_field<4, access_mode::field, Mode> tx_overrun_interrupt_enable;
    bool_read_write_register_field<5, access_mode::field, Mode> rx_overrun_interrupt_enable;
    bool_read_write_register_field<6, access_mode::field, Mode> hs_test_mode;

    raw_register volatile raw;

    constexpr control_register() : raw{0} {}
};
static_assert(sizeof(control_register<>) == 4);

/**
 * @typedef interrupt_register
 * @brief Union for accessing and resetting interrupt register fields
 */
union interrupt_register {
    bool_read_only_register_field<0> tx_interrupt;
    bool_read_only_register_field<1> rx_interrupt;
    bool_read_only_register_field<2> tx_overrun_interrupt;
    bool_read_only_register_field<3> rx_overrun_interrupt;

    bit_write_only_register_field<0> tx_interrupt_reset;
    bit_write_only_register_field<1> rx_interrupt_reset;
    bit_write_only_register_field<2> tx_overrun_interrupt_reset;
    bit_write_only_register_field<3> rx_overrun_interrupt_reset;

    raw_register volatile raw;
};
static_assert(sizeof(interrupt_register) == 4);

/**
 * @typedef bauddiv_register
 * @brief Register for baud rate divisor, minimum 16
 */
using bauddiv_register = read_write_register_field<raw_register, 0, 20, access_mode::bitwise_logic>;
static_assert(sizeof(bauddiv_register) == 4);

/**
 * @struct uart_init
 * @brief Structure for initializing the UART
 */
struct uart_init {
    struct tx_rx {
        bool tx; /**< TX enable flag */
        bool rx; /**< RX enable flag */
    };

    tx_rx         enable;                   /**< Enable TX and RX */
    tx_rx         enable_interrupt;         /**< Enable TX and RX interrupts */
    tx_rx         enable_overrun_interrupt; /**< Enable TX and RX overrun interrupts */
    std::uint32_t baud_rate;                /**< Baud rate */
    bool          enable_hs_test_mode;      /**< Enable high-speed test mode flag for TX */
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

//----------------------------------------------------------------------------
/**
 * @class uart
 * @brief Class representing UART communication device.
 *
 * Non-constructibe, non-copiable, non-movable
 */
class uart {
public:
    uart()            = delete;
    uart(uart const&) = delete;
    uart(uart&&)      = delete;

    uart&
    operator=(uart const&)
        = delete;
    uart&
    operator=(uart&&)
        = delete;

    /**
     * @brief Check if the TX buffer is full
     * @return True if the TX buffer is full, false otherwise
     */
    bool
    tx_buffer_full() const
    {
        return state_.tx_buffer_full;
    }

    /**
     * @brief Check if the RX buffer is full
     * @return True if the RX buffer is full, false otherwise
     */
    bool
    rx_buffer_full() const
    {
        return state_.rx_buffer_full;
    }

    /**
     * @brief Check if the TX buffer has overrun
     * @return True if the TX buffer has overrun, false otherwise
     */
    bool
    tx_buffer_overrun() const
    {
        return state_.tx_buffer_overrun;
    }

    /**
     * @brief Check if the RX buffer has overrun
     * @return True if the RX buffer has overrun, false otherwise
     */
    bool
    rx_buffer_overrun() const
    {
        return state_.rx_buffer_overrun;
    }

    /**
     * @brief Reset the TX buffer overrun flag
     */
    void
    reset_tx_buffer_overrun()
    {
        state_.tx_buffer_overrun = 1;
    }

    /**
     * @brief Reset the RX buffer overrun flag
     */
    void
    reset_rx_buffer_overrun()
    {
        state_.rx_buffer_overrun = 1;
    }

    /**
     * @brief Put a character into the TX buffer
     * @param c The character to put into the TX buffer
     */
    void
    put(char c)    // TODO timeout and error status
    {
        while (tx_buffer_full())
            ;
        data_ = static_cast<raw_register>(c);
    }

    /**
     * @brief Write a string to the TX buffer
     * @param str The null-terminated string to write
     */
    void
    write(char const* str)    // TODO timeout and error status
    {
        for (; *str != 0; ++str) {
            put(*str);
        }
    }

    /**
     * @brief Write an integer value to the TX buffer with a given number base and width
     * @tparam Integer The integer type
     * @param val The integer value to write
     * @param base The number base
     * @param width The width of the output
     * @param fill The fill character for the output
     */
    template <std::integral Integer>
    void
    write(Integer val, number_base base, std::int8_t width, char fill = ' ')
    {
        constexpr auto byte_count = sizeof(Integer);
        constexpr auto bit_count  = byte_count * 8;
        char           buffer[bit_count + 1];
        util::to_chars(buffer, bit_count + 1, val, base, width, fill);
        write(buffer);
    }

    /**
     * @brief Get a character from the RX buffer
     * @return The character from the RX buffer
     */
    char
    get()    // TODO timeout and error status
    {
        while (!tx_buffer_full())
            ;
        return static_cast<char>(data_.get());
    }

private:
    friend class uart_handle;

    /**
     * @brief Configure the UART
     * @param init The initialization parameters
     */
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
 * @class uart_handle
 * @brief Class representing a UART handle
 */
class uart_handle {
public:
    /**
     * @brief Constructor
     * @param device_address The address of the UART device
     */
    uart_handle(address device_address) noexcept : device_{*reinterpret_cast<uart*>(device_address)}
    {}
    /**
     * @brief Constructor with initialization parameters
     * @param device_address The address of the UART device
     * @param init The initialization parameters
     */
    uart_handle(address device_address, uart_init const& init) noexcept
        : uart_handle{device_address}
    {
        configure(init);
    }

    /**
     * @brief Configure the UART with initialization parameters
     * @param init The initialization parameters
     */
    void
    configure(uart_init const& init) noexcept
    {
        // TODO check for error status and report it somehow
        device_.configure(init);
    }

    /**
     * @brief Dereference operator
     * @return The UART device reference
     */
    uart&
    operator*() noexcept
    {
        return device_;
    }

    /**
     * @brief Const dereference operator
     * @return The UART device const reference
     */
    uart const&
    operator*() const noexcept
    {
        return device_;
    }

    /**
     * @brief Arrow operator
     * @return The pointer to the UART device
     */
    uart*
    operator->() noexcept
    {
        return &device_;
    }

    /**
     * @brief Const arrow operator
     * @return The const pointer to the UART device
     */
    uart const*
    operator->() const noexcept
    {
        return &device_;
    }

    /**
     * @brief Set the output number base
     * @param val The output number base to set
     */
    void
    set_output_number_base(number_base val)
    {
        output_number_base_ = val;
    }

    /**
     * @brief Get the output number base
     * @return The current output number base
     */
    number_base
    get_output_number_base() const
    {
        return output_number_base_;
    }

    /**
     * @brief Set the output width
     * @param val The output width to set
     */
    void
    set_output_width(std::uint8_t val)
    {
        output_width_ = val;
    }

    /**
     * @brief Get the output width
     * @return The current output width
     */
    std::uint8_t
    get_output_width() const
    {
        return output_width_;
    }

    /**
     * @brief Set the output fill
     * @param val The output fill to set
     */
    void
    set_output_fill(char val)
    {
        output_fill_ = val;
    }

    /**
     * @brief Get the output fill
     * @return The current output fill
     */
    char
    get_output_fill() const
    {
        return output_fill_;
    }

private:
    uart& device_; /**< The UART device */

    number_base  output_number_base_ = number_base::bin; /**< The output number base */
    std::uint8_t output_width_       = 0;                /**< The output width */
    char         output_fill_        = ' ';              /**< The output fill */
};

//----------------------------------------------------------------------------
// Output
/**
 * @brief Writes a character to the UART handle.
 * @param dev UART device handle
 * @param c   The character to write.
 * @return UART device handle reference
 */
inline uart_handle&
operator<<(uart_handle& dev, char c)
{
    dev->put(c);
    return dev;
}

/**
 * @brief Writes a null-terminated string to the UART handle.
 * @param dev UART device handle
 * @param str The string to write.
 * @return UART device handle reference
 */
inline uart_handle&
operator<<(uart_handle& dev, char const* str)
{
    dev->write(str);
    return dev;
}

/**
 * @brief Writes an integral value to the UART handle.
 * @tparam T The type of the value.
 * @param dev UART device handle
 * @param val The value to write.
 * @return The updated UART handle.
 * @return UART device handle reference
 */
template <std::integral T>
uart_handle&
operator<<(uart_handle& dev, T val)
{
    dev->write(val, dev.get_output_number_base(), dev.get_output_width());
    return dev;
}

/**
 * @brief Writes the value of a readable_field to the UART handle.
 * @tparam F The type of the readable_field.
 * @param dev UART device handle
 * @param reg The readable_field to write.
 * @return UART device handle reference
 */
template <readable_field F>
uart_handle&
operator<<(uart_handle& dev, F const& reg)
{
    dev->write(reg.get(), dev.get_output_number_base(), dev.get_output_width());
    return dev;
}

/**
 * @brief Output manipulator support
 * @param dev UART device handle
 * @param fp The function pointer to call.
 * @return UART device handle reference
 */
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

struct fill_setter {
    char fill_char;

    std::uint8_t target_width;

    friend uart_handle&
    operator<<(uart_handle& dev, fill_setter const& fset)
    {
        dev.set_output_fill(fset.fill_char);
        return dev;
    }
};

}    // namespace detail

/**
 * @brief Sets the output number base to binary.
 * @param dev The UART handle.
 */
inline void
bin_out(uart_handle& dev)
{
    dev.set_output_number_base(number_base::bin);
}

/**
 * @brief Sets the output number base to octal.
 * @param dev The UART handle.
 */
inline void
oct_out(uart_handle& dev)
{
    dev.set_output_number_base(number_base::oct);
}

/**
 * @brief Sets the output number base to decimal.
 * @param dev The UART handle.
 */
inline void
dec_out(uart_handle& dev)
{
    dev.set_output_number_base(number_base::dec);
}

/**
 * @brief Sets the output number base to hexadecimal.
 * @param dev The UART handle.
 */
inline void
hex_out(uart_handle& dev)
{
    dev.set_output_number_base(number_base::hex);
}

/**
 * @brief Sets the output width for the UART handle.
 * @param w The target width.
 */
inline detail::width_setter
width_out(std::uint8_t w)
{
    return {w};
}

/**
 * @brief Sets the output fill for the UART handle.
 * @param c The target fill.
 */
inline detail::fill_setter
fill_out(char c)
{
    return {c};
}

}    // namespace armpp::hal::uart
