#pragma once

#include <armpp/hal/common_types.hpp>
#include <armpp/hal/handle_base.hpp>
#include <armpp/hal/registers.hpp>

// Ref Manual
// https://developer.arm.com/documentation/ddi0337/e/Nested-Vectored-Interrupt-Controller/NVIC-programmer-s-model/NVIC-register-descriptions?lang=en#Cihcajhj

namespace armpp::hal::nvic {

constexpr std::uint32_t interrupt_reg_count = 8;
constexpr std::uint32_t interrupt_count     = 240;    // 240 as per ARM docs

union interrupt_set_enable_register {
    read_only_register_field_array<enabled_t, 1, interrupt_count, interrupt_reg_count> get;
    write_only_register_field_array<set_t, 1, interrupt_count, interrupt_reg_count>    set;
};
static_assert(sizeof(interrupt_set_enable_register) == sizeof(raw_register) * 8);

union interrupt_clear_enable_register {
    read_only_register_field_array<enabled_t, 1, interrupt_count, interrupt_reg_count> get;
    write_only_register_field_array<clear_t, 1, interrupt_count, interrupt_reg_count>  set;
};
static_assert(sizeof(interrupt_clear_enable_register) == sizeof(raw_register) * 8);

union interrupt_set_pending_register {
    read_only_register_field_array<active_t, 1, interrupt_count, interrupt_reg_count> get;
    write_only_register_field_array<set_t, 1, interrupt_count, interrupt_reg_count>   set;
};

union interrupt_clear_pending_register {
    read_only_register_field_array<active_t, 1, interrupt_count, interrupt_reg_count> get;
    write_only_register_field_array<clear_t, 1, interrupt_count, interrupt_reg_count> set;
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
    /**
     * @brief Enable IRQ
     *
     * IRQ cannot be negative
     *
     * @param irqn
     */
    void
    enable_irq(irqn_t irqn)
    {
        auto index       = static_cast<std::uint32_t>(irqn);
        iser_.set[index] = set_t::set;
    }

    void
    disable_irq(irqn_t irqn)
    {
        auto index       = static_cast<std::uint32_t>(irqn);
        icer_.set[index] = clear_t::clear;
    }

    bool
    irq_enabled(irqn_t irqn) const
    {
        auto index = static_cast<std::uint32_t>(irqn);
        return iser_.get[index] == enabled_t::enabled;
    }

    void
    set_pending(irqn_t irqn)
    {
        auto index       = static_cast<std::uint32_t>(irqn);
        ispr_.set[index] = set_t::set;
    }

    void
    clear_pending(irqn_t irqn)
    {
        auto index       = static_cast<std::uint32_t>(irqn);
        icpr_.set[index] = clear_t::clear;
    }

    bool
    is_pending(irqn_t irqn) const
    {
        auto index = static_cast<std::uint32_t>(irqn);
        return ispr_.get[index] == active_t::active;
    }

    bool
    is_active(irqn_t irqn) const
    {
        auto index = static_cast<std::uint32_t>(irqn);
        return iabr_[index] == active_t::active;
    }

    std::uint32_t
    get_irq_priority(irqn_t irq) const;

    void
    set_irq_priority(irqn_t irq, std::uint32_t priority);

    priority_grouping_t
    get_piority_groping() const;

    void set_priority_grouping(priority_grouping_t);
};

static_assert(sizeof(nvic)
              == sizeof(raw_register)
                     * (8 + 24 + 8 + 24 + 8 + 24 + 8 + 24 + 8 + 56 + 60 + 644 + 1));
static_assert(sizeof(nvic) == (0xe000ef04 - nvic::base_address));    // Full NVIC size

class nvic_handle : public handle_base<nvic> {};

// software trigger interrupt                   0xe000ef00

}    // namespace armpp::hal::nvic
