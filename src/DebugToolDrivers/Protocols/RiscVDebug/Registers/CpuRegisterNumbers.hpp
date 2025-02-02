#pragma once

#include "src/DebugToolDrivers/Protocols/RiscVDebug/Common.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebug::Registers
{
    enum class CpuRegisterNumber: RegisterNumber
    {
        DEBUG_CONTROL_STATUS_REGISTER = 0x07B0,
        TRIGGER_SELECT = 0x07A0,
        TRIGGER_DATA_1 = 0x07A1,
        TRIGGER_DATA_2 = 0x07A2,
        TRIGGER_DATA_3 = 0x07A3,
        TRIGGER_INFO = 0x07A4,

        // GPRs
        GPR_X8 = 0x1008,
        GPR_X9 = 0x1009,
    };
}
