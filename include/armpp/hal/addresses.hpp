#pragma once

#include <armpp/hal/common_types.hpp>

namespace armpp::hal {

// TODO Make the header vendor-specific
constexpr address apb1periph_base  = 0x40000000;
constexpr address apb2_periph_base = apb1periph_base + 0x02000;

constexpr address timer0_address = apb1periph_base + 0x0000;
constexpr address timer1_address = apb1periph_base + 0x1000;
constexpr address uart0_address  = apb1periph_base + 0x4000;
constexpr address uart1_address  = apb1periph_base + 0x5000;
constexpr address rtc_address    = apb1periph_base + 0x6000;

}    // namespace armpp::hal
