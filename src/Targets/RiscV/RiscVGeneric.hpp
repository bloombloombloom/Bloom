#pragma once

#include <cstdint>

namespace Targets::RiscV
{
    using RegisterValue = std::uint32_t;
    using RegisterNumber = std::uint16_t;

    enum class PrivilegeMode: std::uint8_t
    {
        U_MODE = 0x00,
        S_MODE = 0x01,
        M_MODE = 0x03,
    };
}
