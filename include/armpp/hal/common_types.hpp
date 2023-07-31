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

}    // namespace armpp::hal
