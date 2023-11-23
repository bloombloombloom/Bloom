#pragma once

#include <cstdint>

#include "src/Targets/RiscV/RiscVGeneric.hpp"

namespace Targets::RiscV::Registers
{
    enum class RegisterNumber: std::uint16_t
    {
        DEBUG_CONTROL_STATUS_REGISTER = 0x07b0,
    };
}
