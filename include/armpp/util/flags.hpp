#pragma once

#include <armpp/util/concepts.hpp>
#include <armpp/util/meta_utility.hpp>

#include <compare>
#include <initializer_list>
#include <type_traits>
#include <utility>

namespace armpp::util {

namespace detail {

template <concepts::enumeration... T>
constexpr std::underlying_type_t<first_type_t<T...>>
or_(T... args) noexcept
    requires(are_same_v<T...>)
{
    using return_type = std::underlying_type_t<first_type_t<T...>>;
    return static_cast<return_type>((static_cast<return_type>(args) | ...));
}

template <concepts::enumeration T, std::size_t... Indexes>
constexpr std::underlying_type_t<T>
or_(std::initializer_list<T> const& vs, std::index_sequence<Indexes...>) noexcept
{
    return or_(std::get<Indexes>(vs)...);
}

template <concepts::enumeration T>
constexpr std::underlying_type_t<T>
or_(std::initializer_list<T> const& vs) noexcept
{
    return or_(vs, std::make_index_sequence<vs.size()>{});
}

}    // namespace detail

template <concepts::enumeration T>
class flags {
public:
    using enumeration_type = T;
    using underlying_type  = std::underlying_type_t<enumeration_type>;

public:
    constexpr flags() noexcept = default;
    constexpr explicit flags(enumeration_type v) noexcept
        : value_{static_cast<underlying_type>(v)} {};

    template <concepts::enumeration... U>
    constexpr flags(U... args) noexcept
        requires((std::is_convertible_v<T, U> && ...))
        : value_{detail::or_(args...)}
    {}

    constexpr auto
    operator<=>(flags const&) const noexcept
        = default;

    constexpr auto
    operator<=>(flags const& other) volatile const noexcept
    {
        return value_ <=> other.value_;
    }

    constexpr auto
    operator<=>(enumeration_type const& other) const noexcept
    {
        return value_ <=> other;
    }

    constexpr auto
    operator<=>(enumeration_type const& other) volatile const noexcept
    {
        return value_ <=> other;
    }

    constexpr explicit operator bool() const noexcept { return value_ != 0; }
    constexpr underlying_type
    underlying() const noexcept
    {
        return value_;
    }

    constexpr auto
    operator!() const noexcept
    {
        return value_ == 0;
    };

    constexpr enumeration_type
    value() const noexcept
    {
        return static_cast<enumeration_type>(value_);
    }

    constexpr flags
    operator~() const noexcept
    {
        return flags{~value_};
    }

    constexpr flags&
    operator|=(flags const& rhs) noexcept
    {
        value_ |= rhs.value_;
        return *this;
    }

    constexpr flags&
    operator|=(enumeration_type v) noexcept
    {
        value_ |= static_cast<underlying_type>(v);
        return *this;
    }

    constexpr flags&
    operator&=(flags const& rhs) noexcept
    {
        value_ &= rhs.value_;
        return *this;
    }

    constexpr flags&
    operator&=(enumeration_type v) noexcept
    {
        value_ &= static_cast<underlying_type>(v);
        return *this;
    }

    constexpr flags&
    operator^=(flags const& rhs) noexcept
    {
        value_ ^= rhs.value_;
        return *this;
    }
    constexpr flags&
    operator^=(enumeration_type v) noexcept
    {
        value_ ^= static_cast<underlying_type>(v);
        return *this;
    }

    template <std::integral U>
    constexpr flags&
    operator>>=(U v) noexcept
    {
        value_ >>= v;
        return *this;
    }

    template <std::integral U>
    constexpr flags&
    operator<<=(U v) noexcept
    {
        value_ <<= v;
        return *this;
    }

private:
    constexpr flags(underlying_type v) : value_{v} {}

private:
    underlying_type value_{0};
};

template <concepts::enumeration T>
constexpr flags<T>
operator|(flags<T> lhs, flags<T> rhs) noexcept
{
    lhs |= rhs;
    return lhs;
}

template <concepts::enumeration T>
constexpr flags<T>
operator|(flags<T> lhs, T rhs) noexcept
{
    lhs |= rhs;
    return lhs;
}

template <concepts::enumeration T>
constexpr flags<T>
operator|(T lhs, flags<T> rhs) noexcept
{
    return rhs | lhs;
}

template <concepts::enumeration T>
constexpr flags<T>
operator&(flags<T> lhs, flags<T> rhs) noexcept
{
    lhs &= rhs;
    return lhs;
}

template <concepts::enumeration T>
constexpr flags<T>
operator&(flags<T> lhs, T rhs) noexcept
{
    lhs &= rhs;
    return lhs;
}

template <concepts::enumeration T>
constexpr flags<T>
operator&(T lhs, flags<T> rhs) noexcept
{
    return rhs & lhs;
}

template <concepts::enumeration T>
constexpr flags<T>
operator^(flags<T> lhs, flags<T> rhs) noexcept
{
    lhs ^= rhs;
    return lhs;
}

template <concepts::enumeration T>
constexpr flags<T>
operator^(flags<T> lhs, T rhs) noexcept
{
    lhs ^= rhs;
    return lhs;
}

template <concepts::enumeration T>
constexpr flags<T>
operator^(T lhs, flags<T> rhs) noexcept
{
    return rhs ^ lhs;
}

template <concepts::enumeration T, std::integral U>
constexpr flags<T>
operator>>(flags<T> lhs, U rhs) noexcept
{
    return lhs >>= rhs;
}

template <concepts::enumeration T, std::integral U>
constexpr flags<T>
operator<<(flags<T> lhs, U rhs) noexcept
{
    return lhs <<= rhs;
}

template <typename T>
struct is_flags : std::false_type {};

template <typename T>
struct is_flags<flags<T>> : std::true_type {};

template <typename T>
constexpr bool is_flags_v = is_flags<T>::value;

}    // namespace armpp::util

namespace armpp::concepts {

template <typename T>
concept flags = util::is_flags_v<T>;

}    // namespace armpp::concepts
