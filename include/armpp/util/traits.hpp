#pragma once

#include <ratio>
#include <type_traits>

namespace armpp::util::traits {

template <typename T>
struct is_ratio : std::false_type {};

template <std::intmax_t N, std::intmax_t D>
struct is_ratio<std::ratio<N, D>> : std::true_type {};

template <typename T>
constexpr bool is_ratio_v = is_ratio<T>::value;

template <std::intmax_t Pn>
struct static_sign : std::integral_constant<std::intmax_t, (Pn < 0 ? -1 : 1)> {};

template <std::intmax_t Pn>
struct static_abs : std::integral_constant<std::intmax_t, Pn * static_sign<Pn>::value> {};

template <std::intmax_t Pn, intmax_t Qn>
struct static_gcd : static_gcd<Qn, (Pn % Qn)> {};

template <std::intmax_t Pn>
struct static_gcd<Pn, 0> : std::integral_constant<std::intmax_t, static_abs<Pn>::value> {};

template <std::intmax_t Qn>
struct static_gcd<0, Qn> : std::integral_constant<std::intmax_t, static_abs<Qn>::value> {};

template <std::intmax_t Pn, std::intmax_t Qn>
struct static_min : std::integral_constant<std::intmax_t, (Pn < Qn ? Pn : Qn)> {};

template <typename Period1, typename Period2>
struct common_ratio {
private:
    using gcd_num = util::traits::static_gcd<Period1::num, Period2::num>;
    using gcd_den = util::traits::static_gcd<Period1::den, Period2::den>;

public:
    using type = std::ratio<gcd_num::value, (Period1::den / gcd_den::value) * Period2::den>;
};

template <typename Period1, typename Period2>
using common_ratio_t = typename common_ratio<Period1, Period2>::type;

static_assert(static_gcd<100, 1000>::value == 100);
static_assert(std::is_same_v<common_ratio_t<std::kilo, std::mega>, std::kilo>);

}    // namespace armpp::util::traits
