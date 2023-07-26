#include <armpp/hal/system.hpp>

void
system_init()
{
    // init clocks here
    using namespace armpp::hal::system;
    // TODO Move the magic constant somewhere
    clock::mutable_instance().system_frequency(54'000'000);    // 54MHz
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
