#pragma once

#include <armpp/hal/handle_base.hpp>
#include <armpp/hal/registers.hpp>

namespace armpp::hal::systick {

/**
 * @brief Enumeration for clock source options.
 */
enum class clock_source_t { external_clock = 0, core_clock = 1 };
/**
 * @brief Enumeration for count flag options.
 */
enum class count_flag_t { clear = 0, counted_to_0 = 1 };

/**
 * @brief Union representing the control and status register for SysTick.
 */
union control_status_register {
    bool_read_write_register_field<0> enable;         /*<! ENABLE Enable counter */
    bool_read_write_register_field<1> handler_enable; /*<! TICKINT Enable pending SysTick handler */
    /**
     * @brief CLKSOURCE Select the clock source.
     *
     * 0 - external reference clock, 1 - core clock
     */
    read_write_register_field<clock_source_t, 2, 1, access_mode::bitwise_logic> source;
    /**
     * @brief Returns 1 if timer counted to 0 since last time this was read. Clears on read.
     *
     * This bit is cleared on read-only if the MasterType bit in the AHB-AP Control Register is set
     * to 0. Otherwise, the COUNTFLAG bit is not changed by the debugger read.
     *
     * Returns 1 if timer counted to 0 since last time this was read. Clears on read by
     * application of any part of the SysTick Control and Status Register. If read by the debugger
     * using the DAP, this bit is cleared on read-only if the MasterType bit in the AHB-AP Control
     * Register is set to 0. Otherwise, the COUNTFLAG bit is not changed by the debugger read.
     *
     */
    read_write_register_field<count_flag_t, 16, 1, access_mode::bitwise_logic> count_flag;
};

/**
 * @brief Register representing the reload value for SysTick.
 */
using reload_value_register = raw_read_write_register_field<0, 24>;
/**
 * @brief Register representing the current value for SysTick.
 */
using current_value_register = raw_read_only_register_field<0, 24>;

/**
 * @brief Union representing the calibration register for SysTick.
 */
union calibration_register {
    /**
     * @brief Reload value to use for 10ms timing.
     *
     * This value is the Reload value to use for 10ms timing. Depending on the value of SKEW, this
     * might be exactly 10ms or might be the closest value.If this reads as 0, then the calibration
     * value is not known. This is probably because the reference clock is an unknown input from the
     * system or scalable dynamically.
     */
    raw_read_only_register_field<0, 24> ten_ms;
    /**
     * @brief  Flag indicating the calibration value is not exactly 10ms because of clock frequency.
     *
     * 1 = the calibration value is not exactly 10ms because of clock frequency. This could affect
     * its suitability as a software real time clock.
     */
    bit_read_only_register_field<30> skew;
    /**
     * @brief Flag indicating the reference clock is not provided.
     */
    bit_read_only_register_field<31> noref;
};

/**
 * @brieff Class representing the SysTick registers.
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

    /**
     * @brief Check if the SysTick counter is enabled.
     * @return `true` if enabled, `false` otherwise.
     */
    bool
    enabled() const
    {
        return control_status_.enable;
    }
    /**
     * @brief Enable the SysTick counter.
     */
    void
    enable()
    {
        control_status_.enable = true;
    }
    /**
     * @brief Disable the SysTick counter.
     */
    void
    disable()
    {
        control_status_.enable = false;
    }

    /**
     * @brief Check if the SysTick interrupt handler is enabled.
     * @return `true` if enabled, `false` otherwise.
     */
    bool
    handler_enabled() const
    {
        return control_status_.handler_enable;
    }
    /**
     * @brief Enable the SysTick interrupt handler.
     */
    void
    handler_enable()
    {
        control_status_.handler_enable = true;
    }
    /**
     * @brief Disable the SysTick interrupt handler.
     */
    void
    handler_disable()
    {
        control_status_.handler_enable = false;
    }

    /**
     * @brief Get the clock source for the SysTick counter.
     * @return The clock source (external or core clock).
     */
    clock_source_t
    source() const
    {
        return control_status_.source;
    }

    /**
     * @brief Set the clock source for the SysTick counter.
     * @param value The clock source to set.
     */
    void
    set_source(clock_source_t value)
    {
        control_status_.source = value;
    }

    /**
     * @brief Get the count flag of the SysTick counter.
     * @return The count flag value.
     */
    count_flag_t
    count_flag() const
    {
        return control_status_.count_flag;
    }

    /**
     * @brief Get the reload value for the SysTick counter.
     * @return The reload value.
     */
    raw_register
    reload_value() const
    {
        return reload_value_;
    }

    /**
     * @brief Set the reload value for the SysTick counter.
     * @param value The reload value to set.
     */
    void
    set_reload_value(raw_register value)
    {
        reload_value_ = value;
    }

    /**
     * @brief Get the current value of the SysTick counter.
     * @return The current value.
     */
    raw_register
    current_value() const
    {
        return current_value_;
    }

private:
    control_status_register control_status_; /*!< Control and status register. */
    reload_value_register   reload_value_;   /*!< Reload value register. */
    current_value_register  current_value_;  /*!< Current value register. */
    calibration_register    calibration_;    /*!< Calibration register. */
};

static_assert(sizeof(systick) == sizeof(raw_register) * 4);

/**
 * @brief Handle class for SysTick.
 */
class systick_handle : public handle_base<systick> {
public:
    using base_type = handle_base<systick>;

    systick_handle() : base_type{systick::base_address} {}
};

}    // namespace armpp::hal::systick
