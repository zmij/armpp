#include <armpp/hal/system.hpp>
//
#include <armpp/hal/systick.hpp>

#ifndef ARMPP_SYSTEM_FREQUENCY
#    error "ARMPP_SYSTEM_FREQUENCY is not set"
#endif

void
system_init()
{
    // init clocks here
    using namespace armpp::hal::system;
    using namespace armpp::frequency::literals;

    using systick_handle = armpp::hal::systick::systick_handle;

    clock::mutable_instance().system_frequency(ARMPP_SYSTEM_FREQUENCY);
    // start systick counter
    systick_handle systick;
    systick->set_reload_value(clock::instance().ticks_per_millisecond() - 1);
    systick->handler_enable();
    systick->enable();
}

void
system_tick()
{
    using namespace armpp::hal::system;
    clock::mutable_instance().increment_tick();
}

namespace armpp::hal::system {

clock&
clock::mutable_instance()
{

    static clock instance_;
    return instance_;
}

clock const&
clock::instance()
{
    return mutable_instance();
}

}    // namespace armpp::hal::system
