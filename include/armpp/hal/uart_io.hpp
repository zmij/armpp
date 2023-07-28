#pragma once

#include <armpp/frequency.hpp>
#include <armpp/hal/uart.hpp>

namespace armpp::hal::uart {
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
 * @brief Writes an integral value of enumeration to the UART handle.
 * @tparam T The type of the value.
 * @param dev UART device handle
 * @param val The value to write.
 * @return The updated UART handle.
 * @return UART device handle reference
 */
template <concepts::enumeration E>
uart_handle&
operator<<(uart_handle& dev, E val)
{
    using integral_type = std::underlying_type_t<E>;
    dev->write(static_cast<integral_type>(val), dev.get_output_number_base(),
               dev.get_output_width());
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

/**
 * @brief Output frequency in Hertz
 *
 * @param dev UART device handle
 * @param val Frequency value
 * @return UART device handle reference
 */
inline uart_handle&
operator<<(uart_handle& dev, frequency::hertz const& val)
{
    auto base = dev.set_output_number_base(number_base::dec);
    dev << val.count() << "Hz";
    dev.set_output_number_base(base);
    return dev;
}

/**
 * @brief Output picoseconds duration
 *
 * @param dev UART device handle
 * @param val Picoseconds value
 * @return UART device handle reference
 */
inline uart_handle&
operator<<(uart_handle& dev, chrono::picoseconds const& val)
{
    auto base = dev.set_output_number_base(number_base::dec);
    dev << val.count() << "ps";
    dev.set_output_number_base(base);
    return dev;
    return dev;
}

/**
 * @brief Output nanoseconds duration
 *
 * @param dev UART device handle
 * @param val nanoseconds value
 * @return UART device handle reference
 */
inline uart_handle&
operator<<(uart_handle& dev, chrono::nanoseconds const& val)
{
    auto base = dev.set_output_number_base(number_base::dec);
    dev << val.count() << "ns";
    dev.set_output_number_base(base);
    return dev;
    return dev;
}

/**
 * @brief Output microseconds duration
 *
 * @param dev UART device handle
 * @param val Microseconds value
 * @return UART device handle reference
 */
inline uart_handle&
operator<<(uart_handle& dev, chrono::microseconds const& val)
{
    auto base = dev.set_output_number_base(number_base::dec);
    dev << val.count() << "µs";
    dev.set_output_number_base(base);
    return dev;
    return dev;
}

/**
 * @brief Output milliseconds duration
 *
 * @param dev UART device handle
 * @param val Milliseconds value
 * @return UART device handle reference
 */
inline uart_handle&
operator<<(uart_handle& dev, chrono::milliseconds const& val)
{
    auto base = dev.set_output_number_base(number_base::dec);
    dev << val.count() << "ms";
    dev.set_output_number_base(base);
    return dev;
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
