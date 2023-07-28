#pragma once

#include <armpp/hal/handle_base.hpp>
#include <armpp/hal/registers.hpp>

namespace armpp::hal::systick {

enum class clock_source_t { external_clock = 0, core_clock = 1 };
enum class count_flag_t { clear = 0, counted_to_0 = 1 };

union control_status_register {
    bool_read_write_register_field<0> enable;         /*<! Enable counter */
    bool_read_write_register_field<1> handler_enable; /*<! TICKINT Enable pending SysTick handler */
    /**
     * 0 - external reference clock, 1 - core clock
     */
    read_write_register_field<clock_source_t, 2, 1, access_mode::bitwise_logic> source;
    /**
     * @brief
     * Returns 1 if timer counted to 0 since last time this was read. Clears on read by
     * application of any part of the SysTick Control and Status Register. If read by the debugger
     * using the DAP, this bit is cleared on read-only if the MasterType bit in the AHB-AP Control
     * Register is set to 0. Otherwise, the COUNTFLAG bit is not changed by the debugger read.
     *
     */
    read_write_register_field<count_flag_t, 16, 1, access_mode::bitwise_logic> count_flag;
};

using reload_value_register  = raw_read_write_register_field<0, 24>;
using current_value_register = raw_read_only_register_field<0, 24>;

union calibration_register {
    /**
     * @brief
     * This value is the Reload value to use for 10ms timing. Depending on the value of SKEW, this
     * might be exactly 10ms or might be the closest value.If this reads as 0, then the calibration
     * value is not known. This is probably because the reference clock is an unknown input from the
     * system or scalable dynamically.
     */
    raw_read_only_register_field<0, 24> ten_ms;
    /**
     * @brief
     * 1 = the calibration value is not exactly 10ms because of clock frequency. This could affect
     * its suitability as a software real time clock.
     */
    bit_read_only_register_field<30> skew;
    bit_read_only_register_field<31> noref; /*<! 1 == the reference clock is not provided */
};

/**
 * @brief SysTick registers
 *
 */
class systick {
public:
    static constexpr address base_address = 0xe000e010;

public:
    systick()               = delete;
    systick(systick const&) = delete;
    systick(systick&&)      = delete;

    systick&
    operator=(systick const&)
        = delete;
    systick&
    operator=(systick&&)
        = delete;

    bool
    enabled() const
    {
        return control_status_.enable;
    }
    void
    enable()
    {
        control_status_.enable = true;
    }
    void
    disable()
    {
        control_status_.enable = false;
    }

    bool
    handler_enabled() const
    {
        return control_status_.handler_enable;
    }
    void
    handler_enable()
    {
        control_status_.handler_enable = true;
    }
    void
    handler_disable()
    {
        control_status_.handler_enable = false;
    }

    clock_source_t
    source() const
    {
        return control_status_.source;
    }

    void
    set_source(clock_source_t value)
    {
        control_status_.source = value;
    }

    count_flag_t
    count_flag() const
    {
        return control_status_.count_flag;
    }

    raw_register
    reload_value() const
    {
        return reload_value_;
    }

    void
    set_reload_value(raw_register value)
    {
        reload_value_ = value;
    }

    raw_register
    current_value() const
    {
        return current_value_;
    }

private:
    control_status_register control_status_;    // 0xe000e010
    reload_value_register   reload_value_;      // 0xe000e014
    current_value_register  current_value_;     // 0xe000e018
    calibration_register    calibration_;       // 0xe000e01c
};

static_assert(sizeof(systick) == sizeof(raw_register) * 4);

class systick_handle : public handle_base<systick> {
public:
    using base_type = handle_base<systick>;

    systick_handle() : base_type{systick::base_address} {}
};

}    // namespace armpp::hal::systick
