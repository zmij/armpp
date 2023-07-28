#pragma once

#include <armpp/frequency.hpp>

#include <cstdint>

extern "C" void
system_init();
extern "C" void
system_tick();

namespace armpp::hal::system {

class clock {
public:
    using tick_type      = std::uint32_t;
    using frequency_type = frequency::hertz;

    using duration   = chrono::milliseconds;
    using period     = typename duration::period;
    using rep        = typename duration::rep;
    using time_point = std::chrono::time_point<clock, duration>;

public:
    clock(clock const&) = delete;
    clock(clock&&)      = delete;

    void
    increment_tick()
    {
        ++tick_;
    }

    tick_type
    tick() const
    {
        return tick_;
    }

    frequency_type
    system_frequency() const
    {
        return system_frequency_;
    }

    tick_type
    ticks_per_millisecond() const
    {
        return system_frequency_.count() / 1000;
    }

public:
    static clock const&
    instance();

    static time_point
    now()
    {
        return time_point{duration{instance().tick_}};
    }

private:
    friend void ::system_init();
    friend void ::system_tick();

    clock() = default;

    static clock&
    mutable_instance();

    template <typename Period>
    void
    system_frequency(frequency::frequency<Period> const& freq)
    {
        system_frequency_ = freq;
    }

    frequency_type system_frequency_;
    tick_type      tick_;
};

}    // namespace armpp::hal::system
