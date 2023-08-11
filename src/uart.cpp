#include <armpp/hal/uart.hpp>
//
#include <armpp/hal/addresses.hpp>
#include <armpp/hal/system.hpp>

#include <array>
#include <cassert>

namespace armpp::hal::uart {

namespace {

// TODO Move to vendor-specific files
constexpr std::size_t uart_count = 2;

std::array<address, uart_count> uart_devices{uart0_address, uart1_address};

struct uart_handlers {
    uart const*             address;
    uart::tx_callback_type  tx_callback;
    uart::rx_callback_type  rx_callback;
    uart::ovr_callback_type tx_ovr_callback;
    uart::ovr_callback_type rx_ovr_callback;

    uart_handlers()
        : address{nullptr},
          tx_callback{nullptr},
          rx_callback{nullptr},
          tx_ovr_callback{nullptr},
          rx_ovr_callback{nullptr}
    {}
};

std::array<uart_handlers, uart_count> handlers;

uart_handlers&
get_handlers(uart const* device)
{
    for (auto& hndlrs : handlers) {
        if (hndlrs.address == device) {
            return hndlrs;
        } else if (hndlrs.address == nullptr) {
            hndlrs.address = device;
            return hndlrs;
        }
    }
    assert(false);
}

}    // namespace

void
uart::configure(uart_init const& init)
{
    control_register<> new_ctrl;

    new_ctrl.tx_enable                   = init.enable.tx;
    new_ctrl.rx_enable                   = init.enable.rx;
    new_ctrl.tx_interrupt_enable         = init.enable_interrupt.tx;
    new_ctrl.rx_interrupt_enable         = init.enable_interrupt.rx;
    new_ctrl.tx_overrun_interrupt_enable = init.enable_overrun_interrupt.tx;
    new_ctrl.rx_overrun_interrupt_enable = init.enable_overrun_interrupt.rx;
    new_ctrl.hs_test_mode                = init.enable_hs_test_mode;

    data_          = 0;
    state_.raw     = 0;
    ctrl_.raw      = 0;
    interrupt_.raw = 0;
    bauddiv_       = 0;

    ctrl_.raw = 0;
    ctrl_.raw = new_ctrl.raw;
    // TODO replace with required clock control
    bauddiv_ = system::clock::instance().system_frequency().count() / init.baud_rate;
}

void
uart::process_interrupt()
{
    auto&       hndlrs = get_handlers(this);
    uart_handle handle{*this};
    if (rx_interrupt() && hndlrs.rx_callback) {
        clear_rx_interrupt();
        hndlrs.rx_callback(handle, data_.get());
    } else if (tx_interrupt() && hndlrs.tx_callback) {
        clear_tx_interrupt();
        hndlrs.tx_callback(handle);
    }
}

void
uart::process_overrun_interrupt()
{
    auto&       hndlrs = get_handlers(this);
    uart_handle handle{*this};
    if (tx_buffer_overrun() && hndlrs.tx_ovr_callback) {
        hndlrs.tx_ovr_callback(handle);
    } else if (rx_buffer_overrun() && hndlrs.rx_ovr_callback) {
        hndlrs.rx_ovr_callback(handle);
    }
}

void
uart::set_tx_handler(uart::tx_callback_type&& cb)
{
    auto& hndlrs       = get_handlers(this);
    hndlrs.tx_callback = std::move(cb);
}

void
uart::set_rx_handler(uart::rx_callback_type&& cb)
{
    auto& hndlrs       = get_handlers(this);
    hndlrs.rx_callback = std::move(cb);
}

void
uart::set_tx_overrun_handler(uart::ovr_callback_type&& cb)
{
    auto& hndlrs           = get_handlers(this);
    hndlrs.tx_ovr_callback = std::move(cb);
}

void
uart::set_rx_overrun_handler(uart::ovr_callback_type&& cb)
{
    auto& hndlrs           = get_handlers(this);
    hndlrs.rx_ovr_callback = std::move(cb);
}

}    // namespace armpp::hal::uart

// TODO Move to vendor-specific files
extern "C" void
uart0_handler()
{
    using namespace armpp::hal::uart;
    uart_handle handle{armpp::hal::uart0_address};
    handle->process_interrupt();
}

extern "C" void
uart1_handler()
{
    using namespace armpp::hal::uart;
    uart_handle handle{armpp::hal::uart0_address};
    handle->process_interrupt();
}

extern "C" void
uart_ovr_handler()
{
    using namespace armpp::hal::uart;
    for (auto addr : uart_devices) {
        uart_handle handle{addr};
        handle->process_overrun_interrupt();
    }
}
