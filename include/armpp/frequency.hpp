#pragma once

#include <armpp/chrono.hpp>
#include <armpp/util/traits.hpp>

namespace armpp::frequency {

using freq_rep = std::uint32_t;

template <typename Period>
struct frequency;

template <typename T>
struct is_frequency : std::false_type {};

template <typename Period>
struct is_frequency<frequency<Period>> : std::true_type {};

template <typename T>
constexpr static bool is_frequency_v = is_frequency<T>::value;

template <typename T>
using enable_if_frequency = std::enable_if_t<is_frequency_v<T>, T>;

namespace detail {

template <typename Freq1, typename Freq2>
struct frequency_common_type;

template <typename Period1, typename Period2>
struct frequency_common_type<frequency<Period1>, frequency<Period2>> {
    using type = frequency<util::traits::common_ratio_t<Period1, Period2>>;
};

template <typename Period1, typename Period2>
using frequency_common_type_t = typename frequency_common_type<Period1, Period2>::type;

template <typename ToFreq, typename CommonFactor, bool NumIsOne = false, bool DenIsOne = false>
struct frequency_cast_impl {
    template <typename Period>
    static constexpr ToFreq
    cast(frequency<Period> const& freq)
    {
        return ToFreq{freq.count() * static_cast<freq_rep>(CommonFactor::num)
                      / static_cast<freq_rep>(CommonFactor::den)};
    }
};

template <typename ToFreq, typename CommonFactor>
struct frequency_cast_impl<ToFreq, CommonFactor, true, true> {
    template <typename Period>
    static constexpr ToFreq
    cast(frequency<Period> const& freq)
    {
        return ToFreq{freq.count()};
    }
};

template <typename ToFreq, typename CommonFactor>
struct frequency_cast_impl<ToFreq, CommonFactor, true, false> {
    template <typename Period>
    static constexpr ToFreq
    cast(frequency<Period> const& freq)
    {
        return ToFreq{freq.count() / static_cast<freq_rep>(CommonFactor::den)};
    }
};

template <typename ToFreq, typename CommonFactor>
struct frequency_cast_impl<ToFreq, CommonFactor, false, true> {
    template <typename Period>
    static constexpr ToFreq
    cast(frequency<Period> const& freq)
    {
        return ToFreq{freq.count() * static_cast<freq_rep>(CommonFactor::num)};
    }
};

}    // namespace detail

template <typename ToFreq, typename Period>
constexpr enable_if_frequency<ToFreq>
frequency_cast(frequency<Period> const& freq)
{
    using to_period = typename ToFreq::period;
    using cf        = std::ratio_divide<Period, to_period>;
    using impl      = detail::frequency_cast_impl<ToFreq, cf, cf::num == 1, cf::den == 1>;
    return impl::cast(freq);
}

template <typename Period>
struct frequency {
    static_assert(util::traits::is_ratio_v<Period>,
                  "Period must be a specialisation of std::ratio");
    static_assert(Period::den == 1, "Fractional frequency is not implemented");

    using rep    = freq_rep;
    using period = Period;

    using duration_period = std::ratio<period::den, period::num * 1000>;
    using duration        = chrono::duration<duration_period>;

    static constexpr duration single_cycle{1000};

public:
    constexpr frequency() noexcept                 = default;
    constexpr frequency(frequency const&) noexcept = default;
    constexpr frequency(frequency&&) noexcept      = default;

    template <typename Rep, typename = std::enable_if_t<std::is_convertible_v<Rep const&, rep>>>
    explicit constexpr frequency(Rep const& v) noexcept : r_{static_cast<rep>(v)}
    {}

    template <typename Period2>
    constexpr frequency(frequency<Period2> const& freq)
        : r_{frequency_cast<frequency>(freq).count()}
    {}

    ~frequency() noexcept = default;

    frequency&
    operator=(frequency const&)
        = default;

    constexpr rep
    count() const noexcept
    {
        return r_;
    }

    template <typename ToDur>
    constexpr chrono::enable_if_duration<ToDur>
    period_duration() const noexcept
    {
        if (r_ == 0) {
            return ToDur{0};
        }
        return std::chrono::duration_cast<ToDur>(single_cycle) / r_;
    }

    //@{
    /** @name Arithmetic */
    constexpr frequency
    operator+() const noexcept
    {
        return *this;
    }
    constexpr frequency
    operator-() const noexcept
    {
        return frequency{-r_};
    }

    constexpr frequency&
    operator++() noexcept
    {
        ++r_;
        return *this;
    }
    constexpr frequency
    operator++(int) noexcept
    {
        return frequency{r_++};
    }

    constexpr frequency&
    operator--() noexcept
    {
        --r_;
        return *this;
    }
    constexpr frequency
    operator--(int) noexcept
    {
        return frequency{r_--};
    }

    constexpr frequency&
    operator+=(frequency const& f)
    {
        r_ += f.count();
        return *this;
    }
    constexpr frequency&
    operator-=(frequency const& f)
    {
        r_ -= f.count();
        return *this;
    }

    constexpr frequency&
    operator*=(rep const& rhs)
    {
        r_ *= rhs;
        return *this;
    }
    constexpr frequency&
    operator/=(rep const& rhs)
    {
        r_ /= rhs;
        return *this;
    }

    constexpr frequency&
    operator%=(rep const& rhs)
    {
        r_ %= rhs;
        return *this;
    }
    constexpr frequency&
    operator%=(frequency const& f)
    {
        r_ %= f.count();
        return *this;
    }
    //@}

private:
    rep r_;
};

using hertz     = frequency<std::ratio<1>>;
using kilohertz = frequency<std::kilo>;
using megahertz = frequency<std::mega>;
using gigahertz = frequency<std::giga>;

template <typename Period1, typename Period2>
constexpr bool
operator==(frequency<Period1> const& lhs, frequency<Period2> const& rhs)
{
    using freq1 = frequency<Period1>;
    using freq2 = frequency<Period2>;
    using ct    = detail::frequency_common_type_t<freq1, freq2>;
    return ct{lhs}.count() == ct{rhs}.count();
}

template <typename Period1, typename Period2>
constexpr bool
operator<(frequency<Period1> const& lhs, frequency<Period2> const& rhs)
{
    using freq1 = frequency<Period1>;
    using freq2 = frequency<Period2>;
    using ct    = detail::frequency_common_type_t<freq1, freq2>;
    return ct{lhs}.count() < ct{rhs}.count();
}

template <typename Period1, typename Period2>
constexpr detail::frequency_common_type_t<frequency<Period1>, frequency<Period2>>
operator+(frequency<Period1> const& lhs, frequency<Period2> const& rhs)
{
    using freq1 = frequency<Period1>;
    using freq2 = frequency<Period2>;
    using ct    = detail::frequency_common_type_t<freq1, freq2>;
    return ct{ct{lhs}.count() + ct{rhs}.count()};
}

template <typename Period>
constexpr frequency<Period>
operator*(frequency<Period> const& lhs, freq_rep const& rhs)
{
    using result_type = frequency<Period>;
    return result_type{lhs.count() * rhs};
}

template <typename Period>
constexpr frequency<Period>
operator*(freq_rep const& lhs, frequency<Period> const& rhs)
{
    return rhs * lhs;
}

template <typename Period>
constexpr hertz
operator/(frequency<Period> const& lhs, freq_rep const& rhs)
{
    return hertz{frequency_cast<hertz>(lhs).count() / rhs};
}

template <typename Period1, typename Period2>
constexpr freq_rep
operator/(frequency<Period1> const& lhs, frequency<Period2> const& rhs)
{
    using freq1 = frequency<Period1>;
    using freq2 = frequency<Period2>;
    using ct    = detail::frequency_common_type_t<freq1, freq2>;
    return ct{lhs}.count() / ct{rhs}.count();
}

template <typename Period>
constexpr hertz
get_frequency(chrono::duration<Period> const& dur)
{
    constexpr chrono::seconds one_sec{1};
    return hertz{std::chrono::duration_cast<chrono::duration<Period>>(one_sec).count()
                 / dur.count()};
}

inline namespace literals {

inline constexpr hertz operator""_Hz(unsigned long long v)
{
    return hertz{v};
}

inline constexpr kilohertz operator""_KHz(unsigned long long v)
{
    return kilohertz{v};
}

inline constexpr megahertz operator""_MHz(unsigned long long v)
{
    return megahertz{v};
}

inline constexpr gigahertz operator""_GHz(unsigned long long v)
{
    return gigahertz{v};
}

}    // namespace literals

// Static checks
namespace static_tests {

using namespace chrono::literals;

static_assert(std::is_same_v<hertz::duration_period, std::milli>);
static_assert(std::is_same_v<kilohertz::duration_period, std::micro>);
static_assert(std::is_same_v<megahertz::duration_period, std::nano>);
static_assert(std::is_same_v<gigahertz::duration_period, std::pico>);

static_assert(std::is_same_v<hertz::duration, chrono::milliseconds>);
static_assert(std::is_same_v<kilohertz::duration, chrono::microseconds>);
static_assert(std::is_same_v<megahertz::duration, chrono::nanoseconds>);
static_assert(std::is_same_v<gigahertz::duration, chrono::picoseconds>);

static_assert(std::is_same_v<detail::frequency_common_type_t<megahertz, kilohertz>, kilohertz>);

static_assert(480_MHz / 48 == 10_MHz);
static_assert(48_MHz == 48000_KHz);

static_assert(megahertz::single_cycle == 1_us);
static_assert((2_MHz).period_duration<chrono::nanoseconds>() == 500_ns);
static_assert((200_KHz).period_duration<chrono::microseconds>() == 5_us);
static_assert((1000000_Hz).period_duration<chrono::microseconds>() == 1_us);

static_assert(get_frequency(200_ms) == 5_Hz);

}    // namespace static_tests

}    // namespace armpp::frequency
