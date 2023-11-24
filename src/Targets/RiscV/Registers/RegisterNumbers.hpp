#pragma once

#include <cstdint>

#include "src/Targets/RiscV/RiscVGeneric.hpp"

namespace Targets::RiscV::Registers
{
    enum class RegisterNumberBase: ::Targets::RiscV::RegisterNumber
    {
        CSR = 0x0000,
        GPR = 0x1000,
        FPR = 0x1020,
        OTHER = 0xc000,
    };

    enum class RegisterNumber: ::Targets::RiscV::RegisterNumber
    {
        DEBUG_CONTROL_STATUS_REGISTER = 0x07b0,
        DEBUG_PROGRAM_COUNTER_REGISTER = 0x07b1,
        STACK_POINTER_X2 = 0x1002,
    };
}
