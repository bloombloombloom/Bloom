#pragma once

#include <cstdint>

namespace Bloom
{
    enum class AddressType: std::uint8_t
    {
        ABSOLUTE,
        RELATIVE,
    };
}
