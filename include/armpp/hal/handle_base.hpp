#pragma once

#include <armpp/hal/registers.hpp>

namespace armpp::hal {

template <typename Device>
struct handle_base {
    using device_type = Device;

    handle_base(address device_address) noexcept
        : device_{*reinterpret_cast<device_type*>(device_address)}
    {}

    device_type&
    operator*() noexcept
    {
        return device_;
    }

    device_type const&
    operator*() const noexcept
    {
        return device_;
    }

    device_type*
    operator->() noexcept
    {
        return &device_;
    }

    device_type const*
    operator->() const noexcept
    {
        return &device_;
    }

protected:
    device_type& device_;
};

}    // namespace armpp::hal
