#pragma once

#include <armpp/hal/registers.hpp>

namespace armpp::hal::timer {

/**
 * @union control_register
 * @brief Union representing the control register of a timer.
 */
union control_register {
    bool_read_write_register_field<0> enable;           /**< Enable field */
    bool_read_write_register_field<1> ext_enable;       /**< External enable field */
    bool_read_write_register_field<2> ext_clock;        /**< External clock field */
    bool_read_write_register_field<3> interrupt_enable; /**< Interrupt enable field */

    raw_register volatile raw;
};
static_assert(sizeof(control_register) == sizeof(raw_register));

/**
 * @typedef value_register
 * @brief Alias representing the value register of a timer.
 */
using value_register = raw_read_write_register_field<0, 32>;
static_assert(sizeof(value_register) == sizeof(raw_register));

/**
 * @typedef reload_register
 * @brief Alias representing the reload register of a timer.
 */
using reload_register = raw_read_write_register_field<0, 32>;
static_assert(sizeof(reload_register) == sizeof(raw_register));

/**
 * @union interrupt_register
 * @brief Union representing the interrupt register of a timer.
 */
union interrupt_register {
    bool_read_only_register_field<0> set; /*<! Check interrupt */

    bit_write_only_register_field<0> reset; /*<! Clear interrupt */
};
static_assert(sizeof(interrupt_register) == sizeof(raw_register));

/**
 * @enum timer_input
 * @brief Enumeration representing the different timer input sources.
 */
enum class timer_input {
    sys_clock, /*<! System clock input */
    ext_input, /*<! External input */
    ext_clock  /*<! External clock */
};

/**
 * @struct timer_init
 * @brief Struct representing the initialization parameters for a timer.
 */
struct timer_init {
    raw_register value;            /**< Value register */
    raw_register reload;           /**< Reload register */
    bool         enable;           /**< Enable flag */
    bool         interrupt_enable; /**< Interrupt enable flag */
    timer_input  input;            /**< Timer input source */
};

/**
 * @class timer
 * @brief Class representing a timer device.
 */
class timer {
public:
    timer()             = delete;
    timer(timer const&) = delete;
    timer(timer&&)      = delete;

    timer&
    operator=(timer const&)
        = delete;
    timer&
    operator=(timer&&)
        = delete;

public:
    /**
     * @brief Starts the timer.
     */
    void
    start()
    {
        ctrl_.enable = true;
    }

    /**
     * @brief Stops the timer.
     */
    void
    stop()
    {
        ctrl_.enable = false;
    }

    /**
     * @brief Gets the interrupt status of the timer.
     * @return True if interrupt is set, false otherwise.
     */
    bool
    get_interrupt() const
    {
        return interrupt_.set;
    }

    /**
     * @brief Clears the interrupt status of the timer.
     */
    void
    clear_interrupt()
    {
        interrupt_.reset = 1;
    }

    /**
     * @brief Enables interrupts for the timer.
     */
    void
    enable_inrerrupt()
    {
        ctrl_.interrupt_enable = true;
    }

    /**
     * @brief Disables interrupts for the timer.
     */
    void
    disable_iterrupt()
    {
        ctrl_.interrupt_enable = false;
    }

    /**
     * @brief Gets the current value of the timer.
     * @return The value of the timer.
     */
    raw_register
    get_value() const
    {
        return value_.get();
    }

    /**
     * @brief Sets the value of the timer.
     * @param val The value to set.
     */
    void
    set_value(raw_register val)
    {
        value_ = val;
    }

    /**
     * @brief Resets the timer.
     */
    void
    reset()
    {
        value_ = 0;
    }

    /**
     * @brief Gets the reload value of the timer.
     * @return The reload value of the timer.
     */
    raw_register
    get_reload() const
    {
        return reload_.get();
    }

    /**
     * @brief Sets the reload value of the timer.
     * @param val The reload value to set.
     */
    void
    set_reload(raw_register val)
    {
        reload_ = val;
    }

private:
    friend class timer_handle;

    /**
     * @brief Configures the timer with the given initialization parameters.
     * @param init The initialization parameters.
     */
    void    // TODO Error status
    configure(timer_init const&);

private:
    control_register   ctrl_;      /*<! Control register */
    value_register     value_;     /*<! Value register */
    reload_register    reload_;    /*<! Reload register */
    interrupt_register interrupt_; /*<! Interrupt status/clear register */
};

static_assert(sizeof(timer) == sizeof(raw_register) * 4);

/**
 * @class timer_handle
 * @brief Class representing a handle to a timer.
 */
class timer_handle {
public:
    /**
     * @brief Constructor for timer_handle.
     * @param device_address The address of the timer device.
     */
    timer_handle(address device_address) noexcept
        : device_{*reinterpret_cast<timer*>(device_address)}
    {}

    /**
     * @brief Constructor for timer_handle.
     * @param device_address The address of the timer device.
     * @param init The initialization parameters for the timer.
     */
    timer_handle(address device_address, timer_init const& init) noexcept
        : timer_handle{device_address}
    {
        configure(init);
    }

    /**
     * @brief Configures the timer with the given initialization parameters.
     * @param init The initialization parameters.
     */
    void
    configure(timer_init const& init) noexcept
    {
        // TODO check for error status and report it somehow
        device_.configure(init);
    }

    /**
     * @brief Dereference operator
     * @return The Timer device reference
     */
    timer&
    operator*() noexcept
    {
        return device_;
    }

    /**
     * @brief Const dereference operator
     * @return The Timer device const reference
     */
    timer const&
    operator*() const noexcept
    {
        return device_;
    }

    /**
     * @brief Arrow operator
     * @return The pointer to the Timer device
     */
    timer*
    operator->() noexcept
    {
        return &device_;
    }

    /**
     * @brief Const arrow operator
     * @return The const pointer to the Timer device
     */
    timer const*
    operator->() const noexcept
    {
        return &device_;
    }

    /**
     * @brief Delays the execution for the specified number of timer ticks.
     * @param ticks The number of timer ticks to delay for.
     */
    void
    delay(std::uint32_t ticks)
    {
        device_.stop();
        device_.reset();
        device_.enable_inrerrupt();

        device_.set_reload(ticks);
        device_.start();

        while (!device_.get_interrupt())
            ;

        device_.stop();
        device_.disable_iterrupt();
        device_.clear_interrupt();
        device_.reset();
    }

private:
    timer& device_; /*<! Reference to the Timer device. */
};

}    // namespace armpp::hal::timer
