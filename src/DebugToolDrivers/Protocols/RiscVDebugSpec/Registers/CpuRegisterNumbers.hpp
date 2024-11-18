#pragma once

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/Common.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec::Registers
{
    enum class CpuRegisterNumber: RegisterNumber
    {
        DEBUG_CONTROL_STATUS_REGISTER = 0x07b0,
        TRIGGER_SELECT = 0x07a0,
        TRIGGER_DATA_1 = 0x07a1,
        TRIGGER_DATA_2 = 0x07a2,
        TRIGGER_DATA_3 = 0x07a3,
        TRIGGER_INFO = 0x07a4,
    };
}
