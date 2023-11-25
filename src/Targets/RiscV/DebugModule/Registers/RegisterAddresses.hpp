#pragma once

#include <cstdint>

#include "src/Targets/RiscV/DebugModule/DebugModule.hpp"

namespace Targets::RiscV::DebugModule::Registers
{
    enum class RegisterAddress: ::Targets::RiscV::DebugModule::RegisterAddress
    {
        ABSTRACT_DATA_0 = 0x04,
        ABSTRACT_DATA_1 = 0x05,
        ABSTRACT_DATA_2 = 0x06,
        ABSTRACT_DATA_3 = 0x07,
        ABSTRACT_DATA_4 = 0x08,
        ABSTRACT_DATA_5 = 0x09,
        ABSTRACT_DATA_6 = 0x0a,
        CONTROL_REGISTER = 0x10,
        STATUS_REGISTER = 0x11,
        ABSTRACT_CONTROL_STATUS_REGISTER = 0x16,
        ABSTRACT_COMMAND_REGISTER = 0x17,
    };
}
