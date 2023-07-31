#include <armpp/hal/nvic.hpp>
//
#include <armpp/hal/scb.hpp>

namespace armpp::hal::scb {

constexpr system_handler_index_t
irqn_to_index(irqn_t irqn)
{
    return static_cast<system_handler_index_t>((static_cast<std::uint32_t>(irqn) & 0xf) - 4);
};

static_assert(irqn_to_index(irqn::memory_management) == system_handler_index_t::mem_manage_fault);
static_assert(irqn_to_index(irqn::bus_fault) == system_handler_index_t::bus_fault);

}    // namespace armpp::hal::scb

namespace armpp::hal::nvic {

namespace {

scb::scb_handle scb_handle;

}    // namespace

std::uint32_t
nvic::get_irq_priority(irqn_t irq) const
{
    if (irq < irqn_t::base) {
        // TODO shift grouping bits. Priority grouping?
        return scb_handle->get_priority(scb::irqn_to_index(irq));
    } else {
        // TODO shift grouping bits
        return ip_[static_cast<std::uint32_t>(irq)];
    }
}

void
nvic::set_irq_priority(irqn_t irq, std::uint32_t priority)
{
    // TODO shift grouping bits. Priority grouping?
    if (irq < irqn_t::base) {
        scb_handle->set_piority(scb::irqn_to_index(irq), priority);
    } else {
        ip_[static_cast<std::uint32_t>(irq)] = priority;
    }
}

priority_grouping_t
nvic::get_piority_groping() const
{
    return scb_handle->get_priority_grouping();
}

void
nvic::set_priority_grouping(priority_grouping_t val)
{
    scb_handle->set_priority_grouping(val);
}

}    // namespace armpp::hal::nvic
