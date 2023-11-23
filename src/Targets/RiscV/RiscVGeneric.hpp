#pragma once

#include <cstdint>

#include "Registers/RegisterNumbers.hpp"

namespace Targets::RiscV
{
    using RegisterValue = std::uint32_t;

    enum class PrivilegeMode: std::uint8_t
    {
        U_MODE = 0x00,
        S_MODE = 0x01,
        M_MODE = 0x03,
    };
}
