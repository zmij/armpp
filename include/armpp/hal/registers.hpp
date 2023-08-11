#pragma once

#include <armpp/hal/common_types.hpp>
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

/**
 * @enum register_mode
 * @brief Enumeration for register type modes.
 *
 * This enumeration defines the register modes: volatile_reg for accessing actual registers and
 * non_volatile_reg for initializing a register value and then using the value to assign to the
 * actual register. In the latter case volatile is of no use thus the data member is not marked
 * volatile. The resulting assembler code for non-volatile register is dramatically shorter.
 */
enum class register_mode { volatile_reg = 0, non_volatile_reg = 1 };

/**
 * @enum access_mode
 * @brief Enumeration for register fields access modes.
 *
 * This enumeration defines the access modes: field and bitwise_logic.
 * In bitwize mode bit fields are used like
 *
 * ```c++
 *  struct field_access_reg {
 *      field_access_reg&
 *      operator = (std::uint32_t val)
 *      {
 *          field_ = val;
 *          return *this;
 *      }
 *
 *      operator std::uint32_t() const {
 *          return field_;
 *      }
 *  private:
 *      std::uint32_t : 5;                  // padding
 *      std::uint32_t volatile field_ : 3;  // the value
 *  };
 * ```
 * Some registers do not allow accessing individual bits, so for manipulating register fields
 * shifting and bitwise logic should be employed. The resulting assembler code differs only slightly
 * for field access and for shift and bitwise and access.
 *
 * This is a simplified implementation of register in masked mode:
 *
 * ```c++
 *  struct masked_reg {
 *      masked_reg&
 *      operator = (std::int32_t val)
 *      {
 *          reg_ |= (val << 5) & mask;
 *          return *this;
 *      }
 *
 *      operator std::uint32_t() const
 *      {
 *          return (reg_ & mask) >> 5;
 *      }
 *  private:
 *      static constexpr std::uint32_t mask = bit_mask_v<5, 3>;
 *      std::uint32_t volatile reg_; // The whole register
 *  };
 * ```
 *
 * This code assings same values to the register fields:
 *
 * ```c++
 * field_access_reg a;
 * masked_reg b;
 * a = 7;
 * b = 7;
 * ```
 *
 * This is ARM GCC assember code for field setting via overloaded operator=() :
 *
 * ```asm
 *      ldrb    r3, [sp, #4]    @ zero_extendqisi2
 *      orr     r3, r3, #224
 *      strb    r3, [sp, #4]
 * ```
 * This is ARM GCC assember code for field setting via overloaded operator=() in a masked register
 * mode:
 *
 * ```asm
 *      ldr     r3, [sp, #4]
 *      orr     r3, r3, #224
 *      str     r3, [sp, #4]
 * ```
 * The code was compiled with -O3 flag
 */
enum class access_mode { field = 0, bitwise_logic };

template <typename T>
struct default_access_mode;

template <concepts::integral T>
struct default_access_mode<T> : std::integral_constant<access_mode, access_mode::field> {};
template <concepts::enumeration T>
struct default_access_mode<T> : std::integral_constant<access_mode, access_mode::bitwise_logic> {};

template <concepts::register_value T>
constexpr access_mode default_access_mode_v = default_access_mode<T>::value;

namespace detail {

/**
 * @brief Template struct for determining the storage type of a register field
 * @tparam T The type of the register value
 * @tparam Mode The mode of the register
 */
template <concepts::register_value T, register_mode Mode>
struct field_storage_type {
    using type = T volatile;
};

/**
 * @brief Specialization of field_storage_type for non-volatile registers
 * @tparam T The type of the register value
 */
template <concepts::register_value T>
struct field_storage_type<T, register_mode::non_volatile_reg> {
    using type = T;
};

/**
 * @brief Alias template for the field storage type
 * @tparam T The type of the register value
 * @tparam Mode The mode of the register
 */
template <concepts::register_value T, register_mode Mode>
using field_storage_type_t = field_storage_type<T, Mode>::type;

/**
 * @brief Template struct for register data manipulation
 * @tparam T The type of the register value
 * @tparam Offset The bit offset of the register value
 * @tparam Size The size in bits of the register value
 * @tparam Access The access mode of the register
 * @tparam Mode The mode of the register
 * @tparam SetValueType The type to use to set the value, defaults to value type
 */
template <concepts::register_value T, std::size_t Offset, std::size_t Size, access_mode Access,
          register_mode Mode, concepts::register_value SetValueType>
struct register_data {
    using value_type     = T;
    using set_value_type = SetValueType;
    using storage_type   = field_storage_type_t<T, Mode>;

    raw_register pad_ : Offset = 0;    ///< Padding bits
    storage_type value_ : Size;        ///< Value of the register field

    /**
     * @brief Get the value of the register
     * @return The value of the register
     */
    constexpr value_type
    get() const
    {
        return value_;
    }

    /**
     * @brief Get the value of the register
     * @return The value of the register
     */
    value_type
    get() volatile const
    {
        return value_;
    }

    /**
     * @brief Set the value of the register
     * @param value The value to be set
     */
    void
    set(set_value_type value)
    {
        if constexpr (!std::is_same_v<value_type, set_value_type>) {
            value_ = static_cast<value_type>(value);
        } else {
            value_ = value;
        }
    }

    /**
     * @brief Set the value of the register
     * @param value The value to be set
     */
    void
    set(set_value_type value) volatile
    {
        if constexpr (!std::is_same_v<value_type, set_value_type>) {
            value_ = static_cast<value_type>(value);
        } else {
            value_ = value;
        }
    }

    constexpr register_data() = default;
};

/**
 * @brief Specialization of register_data for field access mode with zero offset
 * @tparam T The type of the register value
 * @tparam Size The size in bits of the register value
 * @tparam Mode The mode of the register
 * @tparam SetValueType The type to use to set the value, defaults to value type
 */
template <concepts::register_value T, std::size_t Size, register_mode Mode,
          concepts::register_value SetValueType>
struct register_data<T, 0, Size, access_mode::field, Mode, SetValueType> {
    using value_type     = T;
    using set_value_type = SetValueType;
    using storage_type   = field_storage_type_t<T, Mode>;

    storage_type value_ : Size;

    /**
     * @brief Get the value of the register
     * @return The value of the register
     */
    constexpr value_type
    get() const
    {
        return value_;
    }
    value_type
    get() volatile const
    {
        return value_;
    }

    /**
     * @brief Set the value of the register
     * @param value The value to be set
     */
    void
    set(set_value_type value)
    {
        if constexpr (!std::is_same_v<value_type, set_value_type>) {
            value_ = static_cast<value_type>(value);
        } else {
            value_ = value;
        }
    }

    /**
     * @brief Set the value of the register
     * @param value The value to be set
     */
    void
    set(set_value_type value) volatile
    {
        if constexpr (!std::is_same_v<value_type, set_value_type>) {
            value_ = static_cast<value_type>(value);
        } else {
            value_ = value;
        }
    }

    constexpr register_data() = default;
};

/**
 * @brief Specialization of register_data for bitwise logic access mode
 * @tparam T The type of the register value
 * @tparam Offset The bit offset of the register value
 * @tparam Size The size in bits of the register value
 * @tparam Mode The mode of the register
 * @tparam SetValueType The type to use to set the value, defaults to value type
 */
template <concepts::register_value T, std::size_t Offset, std::size_t Size, register_mode Mode,
          concepts::register_value SetValueType>
struct register_data<T, Offset, Size, access_mode::bitwise_logic, Mode, SetValueType> {
    static constexpr auto mask = util::bit_mask_v<Offset, Size, raw_register>;
    using value_type           = T;
    using set_value_type       = SetValueType;
    using storage_type         = field_storage_type_t<raw_register, Mode>;

    storage_type register_;

    /**
     * @brief Get the value of the register
     * @return The value of the register
     */
    constexpr value_type
    get() const
    {
        if constexpr (!std::is_same_v<value_type, raw_register>) {
            return static_cast<value_type>((register_ & mask) >> Offset);
        } else {
            return (register_ & mask) >> Offset;
        }
    }

    value_type
    get() volatile const
    {
        if constexpr (!std::is_same_v<value_type, raw_register>) {
            return static_cast<value_type>((register_ & mask) >> Offset);
        } else {
            return (register_ & mask) >> Offset;
        }
    }

    /**
     * @brief Set the value of the register
     * @param value The value to be set
     */
    void
    set(set_value_type value)
    {
        if constexpr (!std::is_same_v<set_value_type, raw_register>) {
            register_ |= (static_cast<raw_register>(value) << Offset) & mask;
        } else {
            register_ |= (value << Offset) & mask;
        }
    }

    /**
     * @brief Set the value of the register
     * @param value The value to be set
     */
    void
    set(set_value_type value) volatile
    {
        if constexpr (!std::is_same_v<set_value_type, raw_register>) {
            register_ |= (static_cast<raw_register>(value) << Offset) & mask;
        } else {
            register_ |= (value << Offset) & mask;
        }
    }

    constexpr register_data() = default;
};

}    // namespace detail

/**
 *  @struct register_field_base
 *  @brief Template structure representing a register field base class.
 *
 *  This template structure represents a register base with the given type, offset, size, access
 *  mode, and register mode.
 *
 *  It provides common operator overloads and member functions to manipulate the register.
 *
 *  @tparam T Type that satisfies the register_value concept.
 *  @tparam Offset Offset of the register.
 *  @tparam Size Size of the register.
 *  @tparam Access Access mode of the register.
 *  @tparam Mode Register mode (Default: register_mode::volatile_reg).
 *  @tparam SetValueType The type to use to set the value, defaults to value type
 */
template <concepts::register_value T, std::size_t Offset, std::size_t Size,
          access_mode              Access       = default_access_mode_v<T>,
          register_mode            Mode         = register_mode::volatile_reg,
          concepts::register_value SetValueType = T>
    requires(Offset + Size <= register_bits)
struct register_field_base
    : private detail::register_data<T, Offset, Size, Access, Mode, SetValueType> {
    using value_type     = T;
    using set_value_type = SetValueType;

    /** @brief Default constructor. */
    constexpr register_field_base() = default;
    /** @brief Copy constructor (deleted). */
    register_field_base(register_field_base const&) = delete;
    /** @brief Move constructor (deleted). */
    register_field_base(register_field_base&&) = delete;

protected:
    /**
     * @brief Equality operator between two register_field_base instances.
     *
     * @param other The other register_field_base instance to compare.
     * @return      True if the two instances are equal, false otherwise.
     */
    constexpr bool
    operator==(register_field_base const& other) const
    {
        return get() == other.get();
    }
    /**
     * @brief Equality operator between a register_field_base instance and a value.
     *
     * @param other   The value to compare.
     * @return        True if the instance and the value are equal, false otherwise.
     */
    constexpr bool
    operator==(value_type const& other) const
    {
        return get() == get();
    }

    /**
     * @brief Inequality operator between two register_field_base instances.
     *
     * @param other   The other register_field_base instance to compare.
     * @return        True if the two instances are not equal, false otherwise.
     */
    constexpr bool
    operator!=(register_field_base const& other) const
    {
        return !(*this == other);
    }

    /**
     * @brief Inequality operator between a register_field_base instance and a value.
     *
     * @param other   The value to compare.
     * @return        True if the instance and the value are not equal, false otherwise.
     */
    constexpr bool
    operator!=(value_type const& other) const
    {
        return !(*this == other);
    }

    /**
     * @brief Less than operator between two register_field_base instances.
     *
     * @param other   The other register_field_base instance to compare.
     * @return        True if the instance is less than the other instance, false otherwise.
     */
    constexpr bool
    operator<(register_field_base const& other) const
    {
        return get() < other.get();
    }

    /**
     * @brief Less than operator between a register_field_base instance and a value.
     *
     * @param other   The value to compare.
     * @return        True if the instance is less than the value, false otherwise.
     */
    constexpr bool
    operator<(value_type const& other) const
    {
        return get() < other;
    }

    /**
     * @brief Copy assignment operator (deleted).
     */
    register_field_base&
    operator=(register_field_base const&)
        = delete;
    /**
     * @brief Move assignment operator (deleted).
     */
    register_field_base&
    operator=(register_field_base&&)
        = delete;

    /**
     * @brief Assignment operator from a value.
     *
     * @param value   The value to assign.
     * @return        The reference to this register_field_base instance.
     */
    register_field_base&
    operator=(set_value_type const& value)
    {
        set(value);
        return *this;
    }

    /**
     * @brief Assignment operator from a value.
     *
     * @param value   The value to assign.
     * @return        void
     */
    // register_field_base volatile&
    void
    operator=(set_value_type const& value) volatile
    {
        set(value);
        // return *this;
    }

    /**
     * @brief Conversion operator to the value_type.
     *
     * @return        The value of the register.
     */
    constexpr operator value_type() const { return get(); }

    operator value_type() volatile const { return get(); }

    using reg_data_type = detail::register_data<T, Offset, Size, Access, Mode, SetValueType>;
    using reg_data_type::get;
    using reg_data_type::set;
};

template <concepts::register_value T, std::size_t Offset, std::size_t Size, access_mode Access,
          register_mode Mode>
constexpr bool
operator==(T const& rhs, register_field_base<T, Offset, Size, Access, Mode> const& lhs)
{
    return lhs == rhs;
}

template <concepts::register_value T, std::size_t Offset, std::size_t Size, access_mode Access,
          register_mode Mode>
constexpr bool
operator<(T const& rhs, register_field_base<T, Offset, Size, Access, Mode> const& lhs)
{
    return !(lhs < rhs) && (rhs != lhs);
}

/**
 *  @struct read_write_register_field
 *  @brief Template structure representing a read-write register field.
 *
 *  This template structure represents a read-write register with the given type, offset, size,
 *  access mode, and register mode.
 *
 *  It inherits from the register_field_base structure and provides common operator overloads and
 * member functions to manipulate the register.
 *
 *  @tparam T Type that satisfies the register_value concept.
 *  @tparam Offset Offset of the register.
 *  @tparam Size Size of the register.
 *  @tparam Access Access mode of the register.
 *  @tparam Mode Register mode.
 *  @tparam SetValueType The type to use to set the value, defaults to value type
 */
template <concepts::register_value T, std::size_t Offset, std::size_t Size,
          access_mode              Access       = default_access_mode_v<T>,
          register_mode            Mode         = register_mode::volatile_reg,
          concepts::register_value SetValueType = T>
struct read_write_register_field
    : register_field_base<T, Offset, Size, Access, Mode, SetValueType> {
    using base_type  = register_field_base<T, Offset, Size, Access, Mode, SetValueType>;
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

/**
 * @struct read_only_register_field
 * @brief Represents a read-only register field.
 *
 * Provides only read and compare functionality.
 *
 * @tparam T Type of the register value.
 * @tparam Offset Offset of the register.
 * @tparam Size Size of the register.
 * @tparam Access Access mode of the register.
 * @tparam Mode Mode of the register (default: register_mode::volatile_reg).
 * @tparam SetValueType The type to use to set the value, defaults to value type
 */
template <concepts::register_value T, std::size_t Offset, std::size_t Size,
          access_mode              Access       = default_access_mode_v<T>,
          register_mode            Mode         = register_mode::volatile_reg,
          concepts::register_value SetValueType = T>
struct read_only_register_field : register_field_base<T, Offset, Size, Access, Mode, SetValueType> {
    using base_type  = register_field_base<T, Offset, Size, Access, Mode, SetValueType>;
    using value_type = typename base_type::value_type;

    using base_type::base_type;
    using base_type::operator==;
    using base_type::operator!=;
    using base_type::operator<;
    using base_type::get;
    using base_type::operator value_type;
};

/**
 * @struct write_only_register_field
 * @brief Represents a write-only register field.
 *
 * Provides only write functionality, no reading or comparing.
 *
 * @tparam T Type of the register value.
 * @tparam Offset Offset of the register.
 * @tparam Size Size of the register.
 * @tparam Access Access mode of the register.
 * @tparam Mode Mode of the register (default: register_mode::volatile_reg).
 * @tparam SetValueType The type to use to set the value, defaults to value type
 */
template <concepts::register_value T, std::size_t Offset, std::size_t Size,
          access_mode              Access       = default_access_mode_v<T>,
          register_mode            Mode         = register_mode::volatile_reg,
          concepts::register_value SetValueType = T>
struct write_only_register_field
    : register_field_base<T, Offset, Size, Access, Mode, SetValueType> {
    using base_type  = register_field_base<T, Offset, Size, Access, Mode, SetValueType>;
    using value_type = typename base_type::value_type;

    using base_type::base_type;
    using base_type::set;
    using base_type::operator=;
};

/**
 * @typedef raw_read_write_register_field
 * @brief Alias for read_write_register_field with raw_register value type.
 * @tparam Offset Offset of the register.
 * @tparam Size Size of the register.
 * @tparam Access Access mode of the register (default: access_mode::field).
 * @tparam Mode Mode of the register (default: register_mode::volatile_reg).
 */
template <std::size_t Offset, std::size_t Size,
          access_mode   Access = default_access_mode_v<raw_register>,
          register_mode Mode   = register_mode::volatile_reg>
using raw_read_write_register_field
    = read_write_register_field<raw_register, Offset, Size, Access, Mode>;
/**
 * @typedef raw_read_only_register_field
 * @brief Alias for read_only_register_field with raw_register value type.
 * @tparam Offset Offset of the register.
 * @tparam Size Size of the register.
 * @tparam Access Access mode of the register (default: access_mode::field).
 * @tparam Mode Mode of the register (default: register_mode::volatile_reg).
 */
template <std::size_t Offset, std::size_t Size,
          access_mode   Access = default_access_mode_v<raw_register>,
          register_mode Mode   = register_mode::volatile_reg>
using raw_read_only_register_field
    = read_only_register_field<raw_register, Offset, Size, Access, Mode>;
/**
 * @typedef raw_write_only_register_field
 * @brief Alias for write_only_register_field with raw_register value type.
 * @tparam Offset Offset of the register.
 * @tparam Size Size of the register.
 * @tparam Access Access mode of the register (default: access_mode::field).
 * @tparam Mode Mode of the register (default: register_mode::volatile_reg).
 */
template <std::size_t Offset, std::size_t Size,
          access_mode   Access = default_access_mode_v<raw_register>,
          register_mode Mode   = register_mode::volatile_reg>
using raw_write_only_register_field
    = write_only_register_field<raw_register, Offset, Size, Access, Mode>;

/**
 * @typedef bit_read_write_register_field
 * @brief Alias for read_write_register_field with raw_register value type and size 1.
 * @tparam Offset Offset of the register.
 * @tparam Access Access mode of the register (default: access_mode::field).
 * @tparam Mode Mode of the register (default: register_mode::volatile_reg).
 */
template <std::size_t Offset, access_mode Access = default_access_mode_v<raw_register>,
          register_mode Mode = register_mode::volatile_reg>
using bit_read_write_register_field
    = read_write_register_field<raw_register, Offset, 1, Access, Mode>;

/**
 * @typedef bit_read_only_register
 * @brief Alias for read_only_register_field with raw_register value type and size 1.
 * @tparam Offset Offset of the register.
 * @tparam Access Access mode of the register.
 * @tparam Mode Mode of the register (default: register_mode::volatile_reg).
 */
template <std::size_t Offset, access_mode Access = default_access_mode_v<raw_register>,
          register_mode Mode = register_mode::volatile_reg>
using bit_read_only_register_field
    = read_only_register_field<raw_register, Offset, 1, Access, Mode>;

/**
 * @typedef bit_write_only_register
 * @brief Alias for write_only_register_field with raw_register value type and size 1.
 * @tparam Offset Offset of the register.
 * @tparam Access Access mode of the register.
 * @tparam Mode Mode of the register (default: register_mode::volatile_reg).
 */
template <std::size_t Offset, access_mode Access = default_access_mode_v<raw_register>,
          register_mode Mode = register_mode::volatile_reg>
using bit_write_only_register_field
    = write_only_register_field<raw_register, Offset, 1, Access, Mode>;

/**
 * @typedef bit_read_clear_register_field
 *
 * Register to read register bit field, write `clear_t::clear` to clear
 *
 * @tparam Offset Offset of the register.
 * @tparam AccessType Type to read from the register
 * @tparam Access Access mode of the register (default: access_mode::bitwise_logic).
 * @tparam Mode Mode of the register (default: register_mode::volatile_reg).
 */
template <std::size_t   Offset, typename AccessType = raw_register,
          access_mode   Access = access_mode::bitwise_logic,
          register_mode Mode   = register_mode::volatile_reg>
using bit_read_clear_register_field
    = read_write_register_field<AccessType, Offset, 1, Access, Mode, clear_t>;

/**
 * @typedef bool_read_write_register
 * @brief Alias for read_write_register_field with bool value type and size 1.
 * @tparam Offset Offset of the register.
 * @tparam Access Access mode of the register (default: access_mode::field).
 * @tparam Mode Mode of the register (default: register_mode::volatile_reg).
 */
template <std::size_t Offset, access_mode Access = default_access_mode_v<bool>,
          register_mode Mode = register_mode::volatile_reg>
using bool_read_write_register_field = read_write_register_field<bool, Offset, 1, Access, Mode>;

/**
 * @typedef bool_read_only_register
 * @brief Alias for read_only_register_field with bool value type and size 1.
 * @tparam Offset Offset of the register.
 * @tparam Access Access mode of the register (default: access_mode::field).
 * @tparam Mode Mode of the register (default: register_mode::volatile_reg).
 */
template <std::size_t Offset, access_mode Access = default_access_mode_v<bool>,
          register_mode Mode = register_mode::volatile_reg>
using bool_read_only_register_field = read_only_register_field<bool, Offset, 1, Access, Mode>;

/**
 * @typedef bool_write_only_register
 * @brief Alias for write_only_register_field with bool value type and size 1.
 * @tparam Offset Offset of the register.
 * @tparam Access Access mode of the register (default: access_mode::field).
 * @tparam Mode Mode of the register (default: register_mode::volatile_reg).
 */
template <std::size_t Offset, access_mode Access = default_access_mode_v<bool>,
          register_mode Mode = register_mode::volatile_reg>
using bool_write_only_register_field = write_only_register_field<bool, Offset, 1, Access, Mode>;

//----------------------------------------------------------------------------
// Traits and concepts

/**
 * @brief Trait to determine if a type is a register field.
 *
 * This trait is used to check if a given type is a register field or not.
 */
template <typename T>
struct is_register_field : std::false_type {};

/**
 * @brief Trait specialization for register field types.
 *
 * This specialization of `is_register_field` trait is used to check if a given type is a register
 * field. It inherits from `is_base_of` trait to check if the given type is derived from
 * `register_field_base`.
 *
 * @tparam Reg The register type.
 * @tparam T The register value type.
 * @tparam Offset The register field offset.
 * @tparam Size The register field size.
 * @tparam Access The access mode of the register field.
 * @tparam Mode The register mode.
 */
template <template <typename, std::size_t, std::size_t, access_mode, register_mode, typename>
          typename Reg,
          concepts::register_value T, std::size_t Offset, std::size_t Size, access_mode Access,
          register_mode Mode, concepts::register_value SetValueType>
struct is_register_field<Reg<T, Offset, Size, Access, Mode, SetValueType>>
    : std::is_base_of<register_field_base<T, Offset, Size, Access, Mode, SetValueType>,
                      Reg<T, Offset, Size, Access, Mode, SetValueType>> {};

// Traits static test
static_assert(concepts::register_value<bool>);

static_assert(!is_register_field<std::uint32_t>::value);
static_assert(is_register_field<bool_read_write_register_field<0>>::value);
static_assert(is_register_field<bool_read_only_register_field<0>>::value);
static_assert(is_register_field<bool_write_only_register_field<0>>::value);

/**
 * @brief Concept for register fields.
 *
 * This concept is used to define register fields, which are types that satisfy the
 * `is_register_field` trait.
 */
template <typename T>
concept register_field = is_register_field<T>::value;

/**
 * @brief Concept for readable fields.
 *
 * This concept is used to define readable fields, which are register fields that can be read.
 * It requires the `register_field` concept and the `get()` member function.
 *
 * @param reg The register field object.
 */
template <typename T>
concept readable_field = register_field<T> and requires(T const& reg) { reg.get(); };

namespace detail {

template <concepts::register_value T, std::size_t Size>
struct array_field_accessor_base {
    using value_type           = T;
    static constexpr auto mask = util::bit_mask_v<0, Size>;

public:
    array_field_accessor_base() noexcept = default;
    array_field_accessor_base(raw_register volatile& reg, std::size_t offset) noexcept
        : reg_{&reg}, offset_{offset}
    {}

    array_field_accessor_base(array_field_accessor_base const&)     = delete;
    array_field_accessor_base(array_field_accessor_base&&) noexcept = default;

    array_field_accessor_base&
    operator=(array_field_accessor_base const&)
        = delete;
    array_field_accessor_base&
    operator=(array_field_accessor_base&&) noexcept
        = default;

protected:
    // TODO comparison operators
    value_type
    get() const
    {
        if (!reg_)
            // Call usage fault?
            return static_cast<value_type>(0);

        return static_cast<value_type>((*reg_ >> offset_) & mask);
    }

    void
    set(value_type val)
    {
        if (!reg_)
            // Call usage fault?
            return;

        *reg_ |= (static_cast<raw_register>(val) & mask) << offset_;
    }

    operator value_type() const { return get(); }

    array_field_accessor_base&
    operator=(value_type val)
    {
        set(val);
        return *this;
    }

private:
    raw_register volatile* reg_;
    std::size_t            offset_;
};

template <concepts::register_value T, std::size_t Size>
struct array_field_read_write_accessor : array_field_accessor_base<T, Size> {
    using base_type  = array_field_accessor_base<T, Size>;
    using value_type = typename base_type::value_type;

    using base_type::base_type;

    using base_type::get;
    using base_type::operator value_type;

    using base_type::operator=;
    using base_type::set;
};

template <concepts::register_value T, std::size_t Size>
struct array_field_read_only_accessor : array_field_accessor_base<T, Size> {
    using base_type  = array_field_accessor_base<T, Size>;
    using value_type = typename base_type::value_type;

    using base_type::base_type;

    using base_type::get;
    using base_type::operator value_type;
};

template <concepts::register_value T, std::size_t Size>
struct array_field_write_only_accessor : array_field_accessor_base<T, Size> {
    using base_type  = array_field_accessor_base<T, Size>;
    using value_type = typename base_type::value_type;

    using base_type::base_type;

    using base_type::operator=;
    using base_type::set;
};

}    // namespace detail

/**
 * @brief Represents an array of values in a register
 *
 * Base class for accessing or manipulating a register (or a set of continuous registers) as an
 * array of values.
 *
 * For example, NVIC device contains 8 32-bit registers to enable interrupts. It is much more
 * convenient to treat those 8 registers as a single bit array.
 */
template <concepts::register_value T, std::size_t FieldSize, std::size_t FieldCount,
          std::size_t RegisterCount,
          template <concepts::register_value, std::size_t> typename AccessorType,
          std::size_t FieldStorageSize = FieldSize, std::size_t InitialOffset = 0>
    requires(RegisterCount > 0
             && (FieldCount * FieldStorageSize + InitialOffset <= RegisterCount * register_bits))
struct register_field_array_base {
    using value_type           = T;
    static constexpr auto mask = util::bit_mask_v<0, FieldSize>;
    using accessor_type        = AccessorType<value_type, FieldSize>;

protected:
    value_type
    get(std::size_t index) const
    {
        if (index >= FieldCount)
            // Call usage fault?
            return static_cast<value_type>(0);

        auto bit_number = index * FieldStorageSize + InitialOffset;
        auto reg_number = bit_number / register_bits;
        auto reg_offset = bit_number % register_bits;

        return static_cast<value_type>((data_[reg_number] >> reg_offset) & mask);
    }
    accessor_type
    get_accessor(std::size_t index)
    {
        if (index >= FieldCount)
            return accessor_type{};

        auto bit_number = index * FieldStorageSize + InitialOffset;
        auto reg_number = bit_number / register_bits;
        auto reg_offset = bit_number % register_bits;

        return accessor_type{data_[reg_number], reg_offset};
    }

private:
    raw_register volatile data_[RegisterCount];
};

template <concepts::register_value T, std::size_t FieldSize, std::size_t FieldCount,
          std::size_t RegisterCount, std::size_t FieldStorageSize = FieldSize,
          std::size_t InitialOffset = 0>
struct read_write_register_field_array
    : register_field_array_base<T, FieldSize, FieldCount, RegisterCount,
                                detail::array_field_read_write_accessor, FieldStorageSize,
                                InitialOffset> {
    using base_type  = register_field_array_base<T, FieldSize, FieldCount, RegisterCount,
                                                detail::array_field_read_write_accessor,
                                                FieldStorageSize, InitialOffset>;
    using value_type = typename base_type::value_type;

    auto
    operator[](std::size_t index)
    {
        return this->get_accessor(index);
    }

    value_type
    operator[](std::size_t index) const
    {
        return this->get(index);
    }
};

template <concepts::register_value T, std::size_t FieldSize, std::size_t FieldCount,
          std::size_t RegisterCount, std::size_t FieldStorageSize = FieldSize,
          std::size_t InitialOffset = 0>
struct read_only_register_field_array
    : register_field_array_base<T, FieldSize, FieldCount, RegisterCount,
                                detail::array_field_read_only_accessor, FieldStorageSize,
                                InitialOffset> {
    using base_type  = register_field_array_base<T, FieldSize, FieldCount, RegisterCount,
                                                detail::array_field_read_only_accessor,
                                                FieldStorageSize, InitialOffset>;
    using value_type = typename base_type::value_type;

    auto
    operator[](std::size_t index)
    {
        return this->get_accessor(index);
    }

    value_type
    operator[](std::size_t index) const
    {
        return this->get(index);
    }
};

template <concepts::register_value T, std::size_t FieldSize, std::size_t FieldCount,
          std::size_t RegisterCount, std::size_t FieldStorageSize = FieldSize,
          std::size_t InitialOffset = 0>
struct write_only_register_field_array
    : register_field_array_base<T, FieldSize, FieldCount, RegisterCount,
                                detail::array_field_write_only_accessor, FieldStorageSize,
                                InitialOffset> {
    using base_type  = register_field_array_base<T, FieldSize, FieldCount, RegisterCount,
                                                detail::array_field_write_only_accessor,
                                                FieldStorageSize, InitialOffset>;
    using value_type = typename base_type::value_type;

    auto
    operator[](std::size_t index)
    {
        return this->get_accessor(index);
    }
};

}    // namespace armpp::hal
