# armpp

**armpp** is a low-level C++ library designed to simplify the process of 
working with ARM32 hardware. The library is designed to replace traditional
C code with more readable and equally efficient C++ code, providing an
easier way to access and manipulate hardware via user-friendly C++ interfaces.

## Features
The armpp library provides an assortment of classes that allow developers
to interact with ARM32 hardware more conveniently. It's designed to be
compatible with most ARM32 Cortex-M based Microcontrollers (MCUs).

However, it's currently being tested on an ARM32 Cortex-M3 core in a 
Gowin GW1NSR-4C FPGA development board TangNano 4K.

### Register Fields
The library provides building blocks for constructing interfaces for various hardware.

[registers.hpp](include/armpp/hal/registers.hpp) contains templates for describing 
read-write, read-only and write-only register fields. The corresponding templates are:

```c++
template <concepts::register_value T, std::size_t Offset, std::size_t Size, /*...*/ >
struct read_write_register_field;

template <concepts::register_value T, std::size_t Offset, std::size_t Size, /*...*/ >
struct read_only_register_field;

template <concepts::register_value T, std::size_t Offset, std::size_t Size, /*...*/ >
struct write_only_register_field;
```

A couple of shortcut aliases are provided, ACCESSMODE in the following code stands for 
`read_write`, `read_only` and `write_only`:

```c++
/**
 * Integral value of arbitrary size and offset
 */
template <std::size_t Offset, std::size_t Size, /*...*/>
using raw_ACCESSMODE_register_field = ACCESSMODE_register_field<raw_register, Offset, Size, /*...*/>;

/**
 * Access individual bits at arbitrary offsets
 */
template <std::size_t Offset, /*...*/>
using bit_ACCESSMODE_register_field = ACCESSMODE_register_field<raw_register, Offset, 1, /*...*/>;

/**
 * Access individual bits as type boolean
 */
template <std::size_t Offset, std::size_t Size, /*...*/>
using bool_ACCESSMODE_register_field = ACCESSMODE_register_field<bool, Offset, Size, /*...*/>;

```

A register is constructed from the fields using `union`, for example, to implement a simple
status register for a Gowin Cortex-M3 UART device, that contains 4 fields we can write:

```c++
union state_register {
    bool_read_only_register_field<0> tx_buffer_full;
    bool_read_only_register_field<1> rx_buffer_full;
    bool_read_write_register_field<2> tx_buffer_overrun;
    bool_read_write_register_field<3> rx_buffer_overrun;
};
```

The register fields can be used with integral types (signed and unsigned) and enumerations.


## Prerequisites
To use the library, you will need to have the arm-none-eabi toolkit installed. 
I'm currently working on adding support for the clang toolkit. The library is
built using cmake, and instructions for building from source will be provided later.

## Getting Started
Examples and usage guidelines are currently under development and will be added in the future.

```c++
#include <armpp/hal/uart.hpp>

extern "C"
int main()
{
    // This function will be removed later
    system_init();

    armpp::hal::uart::uart_handle uart0{uart0_address, {.enable{.tx = true}, .baud_rate = 9600}};
    uart0 << "Hello world!\r\n";

    while(1) {}
}

```

## Building the Library
The armpp library can be compiled from the source using cmake.

Detailed build instructions will be provided soon.

## Known Issues
Currently, the library is only compatible with the arm-none-eabi toolkit.
We are actively working on adding support for the clang toolkit.

## Contributing
Contributions are welcome! We are currently developing guidelines for contributing,
but if you're eager to contribute, feel free to create a pull request.

## License
[MIT License](LICENSE)

For any issues, questions, or suggestions, please open an issue in the GitHub repository.

*Please note that armpp is a work in progress, and some features may not be fully functional or complete. Check back often for updates, and thanks for your interest!*
