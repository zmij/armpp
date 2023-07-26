#pragma once

#include <armpp/util/mask.hpp>

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace armpp::concepts {

template <typename T>
concept integral = std::is_integral_v<T>;

template <typename T>
concept enumeration = std::is_enum_v<T>;

template <typename T>
concept register_value = integral<T> || enumeration<T>;

}    // namespace armpp::concepts

namespace armpp::hal {

constexpr std::size_t register_bits = 32;

using raw_register = std::uint32_t;
using address      = std::uint32_t;

/**
 * @brief Describes the mode of register, volatile (for actual register) or non-volatile, for init
 * data
 *
 */
enum class register_mode { volatile_reg = 0, non_volatile_reg = 1 };
enum class access_mode { bitwise = 0, register_wise };

namespace detail {

template <concepts::register_value T, std::size_t Offset, std::size_t Size, access_mode Access,
          register_mode Mode>
struct register_data {
    using value_type           = T;
    raw_register pad_ : Offset = 0;
    value_type volatile value_ : Size;

    constexpr value_type
    get() const
    {
        return value_;
    }

    void
    set(value_type value)
    {
        value_ = value;
    }

    constexpr register_data() = default;
};

template <concepts::register_value T, std::size_t Size>
struct register_data<T, 0, Size, access_mode::bitwise, register_mode::volatile_reg> {
    using value_type = T;
    value_type volatile value_ : Size;

    constexpr value_type
    get() const
    {
        return value_;
    }

    void
    set(value_type value)
    {
        value_ = value;
    }

    constexpr register_data() = default;
};

template <concepts::register_value T, std::size_t Offset, std::size_t Size>
struct register_data<T, Offset, Size, access_mode::bitwise, register_mode::non_volatile_reg> {
    using value_type           = T;
    raw_register pad_ : Offset = 0;
    value_type   value_ : Size;

    constexpr value_type
    get() const
    {
        return value_;
    }

    void
    set(value_type value)
    {
        value_ = value;
    }

    constexpr register_data() = default;
};

template <concepts::register_value T, std::size_t Size>
struct register_data<T, 0, Size, access_mode::bitwise, register_mode::non_volatile_reg> {
    using value_type = T;
    value_type value_ : Size;

    constexpr value_type
    get() const
    {
        return value_;
    }

    void
    set(value_type value)
    {
        value_ = value;
    }

    constexpr register_data() = default;
};



template <concepts::register_value T, std::size_t Offset, std::size_t Size>
struct register_data<T, Offset, Size, access_mode::register_wise, register_mode::volatile_reg> {
    using value_type           = T;
    static constexpr auto mask = util::bit_mask_v<Offset, Size, raw_register>;

    constexpr value_type
    get() const
    {
        if constexpr (!std::is_same_v<value_type, raw_register>) {
            return static_cast<value_type>((register_ & mask) >> Offset);
        } else {
            return (register_ & mask) >> Offset;
        }
    }

    void
    set(value_type value)
    {
        if constexpr (!std::is_same_v<value_type, raw_register>) {
            register_ |= (static_cast<raw_register>(value) << Offset) & mask;
        } else {
            register_ |= (value << Offset) & mask;
        }
    }

    raw_register volatile register_;
};

}    // namespace detail

template <concepts::register_value T, std::size_t Offset, std::size_t Size,
          access_mode   Access = access_mode::bitwise,
          register_mode Mode   = register_mode::volatile_reg>
    requires(Offset + Size <= register_bits)
struct register_base : private detail::register_data<T, Offset, Size, Access, Mode> {
    using value_type = T;

    constexpr register_base() = default;

protected:
    constexpr bool
    operator==(register_base const& other) const
    {
        return get() == other.get();
    }
    constexpr bool
    operator==(value_type const& other) const
    {
        return get() == get();
    }

    constexpr bool
    operator!=(register_base const& other) const
    {
        return !(*this == other);
    }

    constexpr bool
    operator!=(value_type const& other) const
    {
        return !(*this == other);
    }

    constexpr bool
    operator<(register_base const& other) const
    {
        return get() < other.get();
    }
    constexpr bool
    operator<(value_type const& other) const
    {
        return get() < other;
    }

    register_base&
    operator=(register_base const&)
        = delete;
    register_base&
    operator=(register_base&&)
        = delete;

    register_base&
    operator=(T const& value)
    {
        set(value);
        return *this;
    }

    constexpr operator value_type() const { return get(); }

    using reg_data_type = detail::register_data<T, Offset, Size, Access, Mode>;
    using reg_data_type::get;
    using reg_data_type::set;

private:
    register_base(register_base const&) = delete;
    register_base(register_base&&)      = delete;
};

template <concepts::register_value T, std::size_t Offset, std::size_t Size, access_mode Access,
          register_mode Mode>
constexpr bool
operator==(T const& rhs, register_base<T, Offset, Size, Access, Mode> const& lhs)
{
    return lhs == rhs;
}

template <concepts::register_value T, std::size_t Offset, std::size_t Size, access_mode Access,
          register_mode Mode>
constexpr bool
operator<(T const& rhs, register_base<T, Offset, Size, Access, Mode> const& lhs)
{
    return !(lhs < rhs) && (rhs != lhs);
}

template <concepts::register_value T, std::size_t Offset, std::size_t Size,
          access_mode   Access = access_mode::bitwise,
          register_mode Mode   = register_mode::volatile_reg>
struct read_write_register : register_base<T, Offset, Size, Access, Mode> {
    using base_type  = register_base<T, Offset, Size, Access, Mode>;
    using value_type = typename base_type::value_type;

    using base_type::base_type;
    using base_type::operator==;
    using base_type::operator!=;
    using base_type::operator<;
    using base_type::get;
    using base_type::operator value_type;
    using base_type::set;
    using base_type::operator=;
};

template <concepts::register_value T, std::size_t Offset, std::size_t Size,
          access_mode   Access = access_mode::bitwise,
          register_mode Mode   = register_mode::volatile_reg>
struct read_only_register : register_base<T, Offset, Size, Access, Mode> {
    using base_type  = register_base<T, Offset, Size, Access, Mode>;
    using value_type = typename base_type::value_type;

    using base_type::base_type;
    using base_type::operator==;
    using base_type::operator!=;
    using base_type::operator<;
    using base_type::get;
    using base_type::operator value_type;
};

template <concepts::register_value T, std::size_t Offset, std::size_t Size,
          access_mode   Access = access_mode::bitwise,
          register_mode Mode   = register_mode::volatile_reg>
struct write_only_register : register_base<T, Offset, Size, Access, Mode> {
    using base_type  = register_base<T, Offset, Size, Access, Mode>;
    using value_type = typename base_type::value_type;

    using base_type::base_type;
    using base_type::set;
    using base_type::operator=;
};

template <std::size_t Offset, std::size_t Size>
using raw_read_write_register = read_write_register<raw_register, Offset, Size>;
template <std::size_t Offset, std::size_t Size>
using raw_read_only_register = read_only_register<raw_register, Offset, Size>;
template <std::size_t Offset, std::size_t Size>
using raw_write_only_register = write_only_register<raw_register, Offset, Size>;

template <std::size_t Offset, access_mode Access = access_mode::bitwise,
          register_mode Mode = register_mode::volatile_reg>
using bit_read_write_register = read_write_register<raw_register, Offset, 1, Access, Mode>;
template <std::size_t Offset, access_mode Access = access_mode::bitwise,
          register_mode Mode = register_mode::volatile_reg>
using bit_read_only_register = read_only_register<raw_register, Offset, 1, Access, Mode>;
template <std::size_t Offset, access_mode Access = access_mode::bitwise,
          register_mode Mode = register_mode::volatile_reg>
using bit_write_only_register = write_only_register<raw_register, Offset, 1, Access, Mode>;

template <std::size_t Offset, access_mode Access = access_mode::bitwise,
          register_mode Mode = register_mode::volatile_reg>
using bool_read_write_register = read_write_register<bool, Offset, 1, Access, Mode>;
template <std::size_t Offset, access_mode Access = access_mode::bitwise,
          register_mode Mode = register_mode::volatile_reg>
using bool_read_only_register = read_only_register<bool, Offset, 1, Access, Mode>;
template <std::size_t Offset, access_mode Access = access_mode::bitwise,
          register_mode Mode = register_mode::volatile_reg>
using bool_write_only_register = write_only_register<bool, Offset, 1, Access, Mode>;

//----------------------------------------------------------------------------
// Traits and concepts
template <typename T>
struct is_register_field : std::false_type {};
template <template <concepts::register_value, std::size_t, std::size_t, access_mode, register_mode>
          typename Reg,
          concepts::register_value T, std::size_t Offset, std::size_t Size, access_mode Access,
          register_mode Mode>
struct is_register_field<Reg<T, Offset, Size, Access, Mode>>
    : std::is_base_of<register_base<T, Offset, Size, Access, Mode>,
                      Reg<T, Offset, Size, Access, Mode>> {};

// Traits static test
static_assert(!is_register_field<std::uint32_t>::value);
static_assert(is_register_field<bool_read_write_register<0>>::value);
static_assert(is_register_field<bool_read_only_register<0>>::value);
static_assert(is_register_field<bool_write_only_register<0>>::value);

template <typename T>
concept register_field = is_register_field<T>::value;

template <typename T>
concept readable_field = register_field<T> and requires(T const& reg) { reg.get(); };

}    // namespace armpp::hal
