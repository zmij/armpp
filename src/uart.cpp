#include <armpp/hal/uart.hpp>
//
#include <armpp/hal/system.hpp>

namespace armpp::hal::uart {
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
    bauddiv_ = system::clock::instance().system_frequency() / init.baud_rate;
}
}    // namespace armpp::hal::uart
