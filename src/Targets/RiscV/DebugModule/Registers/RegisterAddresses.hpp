#pragma once

#include <cstdint>

namespace Targets::RiscV::DebugModule::Registers
{
    enum RegisterAddresses: std::uint8_t
    {
        ABSTRACT_DATA_0 = 0x04,
        CONTROL_REGISTER = 0x10,
        STATUS_REGISTER = 0x11,
        ABSTRACT_CONTROL_STATUS_REGISTER = 0x16,
        ABSTRACT_COMMAND_REGISTER = 0x17,
    };
}
