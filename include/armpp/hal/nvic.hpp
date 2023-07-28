#pragma once

#include <armpp/hal/handle_base.hpp>
#include <armpp/hal/registers.hpp>

// Ref Manual
// https://developer.arm.com/documentation/ddi0337/e/Nested-Vectored-Interrupt-Controller/NVIC-programmer-s-model/NVIC-register-descriptions?lang=en#Cihcajhj

namespace armpp::hal::nvic {

constexpr std::uint32_t interrupt_reg_count = 8;
constexpr std::uint32_t interrupt_count     = 240;    // 240 as per ARM docs

enum class set_t { no_effect = 0, set = 1 };
enum class reset_t { no_effect = 0, reset = 1 };
enum class enabled_t { disabled = 0, enabled = 1 };
enum class active_t { inactive = 0, active = 1 };

union interrupt_set_enable_register {
    write_only_register_field_array<set_t, 1, interrupt_count, interrupt_reg_count>    set;
    read_only_register_field_array<enabled_t, 1, interrupt_count, interrupt_reg_count> get;
};
static_assert(sizeof(interrupt_set_enable_register) == sizeof(raw_register) * 8);

union interrupt_clear_enable_register {
    write_only_register_field_array<reset_t, 1, interrupt_count, interrupt_reg_count>  set;
    read_only_register_field_array<enabled_t, 1, interrupt_count, interrupt_reg_count> get;
};
static_assert(sizeof(interrupt_clear_enable_register) == sizeof(raw_register) * 8);

union interrupt_set_pending_register {
    write_only_register_field_array<set_t, 1, interrupt_count, interrupt_reg_count>   set;
    read_only_register_field_array<active_t, 1, interrupt_count, interrupt_reg_count> get;
};

union interrupt_clear_pending_register {
    write_only_register_field_array<reset_t, 1, interrupt_count, interrupt_reg_count> set;
    read_only_register_field_array<active_t, 1, interrupt_count, interrupt_reg_count> get;
};

using active_bit_register
    = read_only_register_field_array<active_t, 1, interrupt_count, interrupt_reg_count>;

using interrupt_priority_register
    = read_write_register_field_array<std::uint32_t, 8, interrupt_count, interrupt_count / 4>;

using software_trigger_interrupt_register = read_write_register_field_array<active_t, 1, 9, 1>;

/**
 * @brief Structure for NVIC registers
 *
 * This structure contains NVIC registers as public members to check their addresses with offsetof
 *
 */
struct nvic_registers {
    static constexpr address base_address = 0xe000e100;
    static constexpr address iser_base    = base_address;
    static constexpr address icer_base    = 0xe000e180;
    static constexpr address ispr_base    = 0xe000e200;
    static constexpr address icpr_base    = 0xe000e280;
    static constexpr address iabr_base    = 0xe000e300;
    static constexpr address ip_base      = 0xe000e400;
    static constexpr address stir_base    = 0xe000ef00;

    interrupt_set_enable_register       iser_;    // 0xe000e100 - 0xe000e120
    raw_register const                  reserved_0_[24];
    interrupt_clear_enable_register     icer_;    // 0xe000e180 - 0e000e1a0
    raw_register const                  reserved_1_[24];
    interrupt_set_pending_register      ispr_;    // 0xe000e200 - 0xe000e220
    raw_register const                  reserved_2_[24];
    interrupt_clear_pending_register    icpr_;    // 0xe000e280 - 0xe000e2a0
    raw_register const                  reserved_3_[24];
    active_bit_register                 iabr_;    // 0xe000e300 - 0xe000e320
    raw_register const                  reserved_4_[56];
    interrupt_priority_register         ip_;    // 0xe000e400 - 0xe000e41f (?)
    raw_register const                  reserved_5_[644];
    software_trigger_interrupt_register stir_;
};
static_assert(offsetof(nvic_registers, iser_)
              == (nvic_registers::iser_base - nvic_registers::base_address));
static_assert(offsetof(nvic_registers, icer_)
              == (nvic_registers::icer_base - nvic_registers::base_address));
static_assert(offsetof(nvic_registers, ispr_)
              == (nvic_registers::ispr_base - nvic_registers::base_address));
static_assert(offsetof(nvic_registers, icpr_)
              == (nvic_registers::icpr_base - nvic_registers::base_address));
static_assert(offsetof(nvic_registers, iabr_)
              == (nvic_registers::iabr_base - nvic_registers::base_address));
static_assert(offsetof(nvic_registers, ip_)
              == (nvic_registers::ip_base - nvic_registers::base_address));
static_assert(offsetof(nvic_registers, stir_)
              == (nvic_registers::stir_base - nvic_registers::base_address));

class nvic : nvic_registers {
public:
    using nvic_registers::base_address;

public:
    nvic()            = delete;
    nvic(nvic const&) = delete;
    nvic(nvic&&)      = delete;

    nvic&
    operator=(nvic const&)
        = delete;
    nvic&
    operator=(nvic&&)
        = delete;

public:
    void
    enable_irq(std::size_t index)
    {
        iser_.set[index] = set_t::set;
    }

    void
    disable_irq(std::size_t index)
    {
        icer_.set[index] = reset_t::reset;
    }

    bool
    irq_enabled(std::size_t index) const
    {
        return iser_.get[index] == enabled_t::enabled;
    }

    void
    set_pending(std::size_t index)
    {
        ispr_.set[index] = set_t::set;
    }

    void
    clear_pending(std::size_t index)
    {
        icpr_.set[index] = reset_t::reset;
    }

    bool
    is_pending(std::size_t index) const
    {
        return ispr_.get[index] == active_t::active;
    }
};

static_assert(sizeof(nvic)
              == sizeof(raw_register)
                     * (8 + 24 + 8 + 24 + 8 + 24 + 8 + 24 + 8 + 56 + 60 + 644 + 1));
static_assert(sizeof(nvic) == (0xe000ef04 - nvic::base_address));    // Full NVIC size

class systick_handle : public handle_base<nvic> {
public:
    using base_type = handle_base<nvic>;

    systick_handle() : base_type{nvic::base_address} {}
};

// software trigger interrupt                   0xe000ef00

}    // namespace armpp::hal::nvic
