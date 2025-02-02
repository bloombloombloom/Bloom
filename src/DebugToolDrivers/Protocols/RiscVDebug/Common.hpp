#pragma once

#include <cstdint>

namespace DebugToolDrivers::Protocols::RiscVDebug
{
    using RegisterValue = std::uint32_t;
    using RegisterNumber = std::uint16_t;

    enum class PrivilegeMode: std::uint8_t
    {
        USER = 0x00,
        SUPERVISOR = 0x01,
        MACHINE = 0x03,
    };
}
