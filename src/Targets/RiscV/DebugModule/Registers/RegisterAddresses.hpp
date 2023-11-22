#pragma once

#include <cstdint>

namespace Targets::RiscV::DebugModule::Registers
{
    enum RegisterAddresses: std::uint8_t
    {
        CONTROL_REGISTER = 0x10,
        STATUS_REGISTER = 0x11,
    };
}
