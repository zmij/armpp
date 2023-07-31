#pragma once

#include <cstdint>

namespace armpp::hal {

using raw_register                  = std::uint32_t;
constexpr std::size_t register_bits = sizeof(raw_register) * 8;

using address = std::uint32_t;

enum class set_t { no_effect = 0, set = 1 };
enum class clear_t { no_effect = 0, clear = 1 };
enum class enabled_t { disabled = 0, enabled = 1 };
enum class active_t { inactive = 0, active = 1 };
enum class pended_t { not_pended = 0, pended = 1 };

union cpu_id {
    struct {
        raw_register revision : 4;
        raw_register partno : 12;
        raw_register constant : 4;
        raw_register vairant : 4;
        raw_register implementer : 8;
    };
    raw_register raw;
};
static_assert(sizeof(cpu_id) == sizeof(raw_register));

enum class priority_grouping_t {
    split_7_1 = 0, /*<! 7.1 indicates 7 bits of pre-emption priority, 1 bit of subpriority */
    split_6_2 = 1, /*<! 6.2 indicates 6 bits of pre-emption priority, 2 bits of subpriority */
    split_5_3 = 2, /*<! 5.3 indicates 5 bits of pre-emption priority, 3 bits of subpriority */
    split_4_5 = 3, /*<! 5.3 indicates 4 bits of pre-emption priority, 4 bits of subpriority */
    split_3_5 = 4, /*<! 5.3 indicates 3 bits of pre-emption priority, 5 bits of subpriority */
    split_2_6 = 5, /*<! 5.3 indicates 2 bits of pre-emption priority, 6 bits of subpriority */
    split_1_7 = 6, /*<! 5.3 indicates 1 bits of pre-emption priority, 7 bits of subpriority */
    split_0_8 = 7  /*<! 0.8 indicates no pre-emption priority, 8 bits of subpriority.*/
};

enum class irqn_t : std::int32_t { base = 0 };

inline namespace cm3 {

/**
 * @brief Namespace for CM3 processor exceptions
 */
namespace irqn {

constexpr irqn_t non_maskable_int{-14};
constexpr irqn_t hard_fault{-13};
constexpr irqn_t memory_management{-12};
constexpr irqn_t bus_fault{-11};
constexpr irqn_t usage_fault{-10};
constexpr irqn_t svcall{-5};
constexpr irqn_t debug_monitor{-4};
constexpr irqn_t pensv{-2};
constexpr irqn_t systick{-1};

}    // namespace irqn

}    // namespace cm3

}    // namespace armpp::hal
