#include <armpp/hal/timer.hpp>
//

namespace armpp::hal::timer {

void
timer::configure(timer_init const& init)
{
    ctrl_.raw        = 0;
    value_           = 0;
    reload_          = 0;
    interrupt_.reset = 0;    // Is this valid?

    value_  = init.value;
    reload_ = init.reload;

    ctrl_.enable           = true;    // Is this truly necessary?
    ctrl_.interrupt_enable = init.interrupt_enable;

    switch (init.input) {
    case timer_input::sys_clock:
        // Do nothing, ext input fields already cleared
        break;
    case timer_input::ext_input:
        ctrl_.ext_enable = true;
    case timer_input::ext_clock:
        ctrl_.ext_clock = true;
    }

    ctrl_.enable = init.enable;
}

}    // namespace armpp::hal::timer
