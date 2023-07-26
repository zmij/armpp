#include <armpp/hal/system.hpp>

void
system_init()
{
    // init clocks here
}

void
system_tick()
{
    armpp::hal::system::clock::mutable_instance().increment_tick();
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
