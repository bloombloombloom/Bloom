#pragma once

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/RiscVGeneric.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec::Registers
{
    enum class CpuRegisterNumber: RegisterNumber
    {
        DEBUG_CONTROL_STATUS_REGISTER = 0x07b0,
    };
}
