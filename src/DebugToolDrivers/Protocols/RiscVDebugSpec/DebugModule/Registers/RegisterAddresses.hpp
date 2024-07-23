#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/DebugModule/DebugModule.hpp"

namespace DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::Registers
{
    enum class RegisterAddress: RegisterAddress
    {
        ABSTRACT_DATA_0 = 0x04,
        ABSTRACT_DATA_1 = 0x05,
        ABSTRACT_DATA_2 = 0x06,
        ABSTRACT_DATA_3 = 0x07,
        ABSTRACT_DATA_4 = 0x08,
        ABSTRACT_DATA_5 = 0x09,
        ABSTRACT_DATA_6 = 0x0a,
        ABSTRACT_DATA_7 = 0x0b,
        ABSTRACT_DATA_8 = 0x0c,
        ABSTRACT_DATA_9 = 0x0d,
        ABSTRACT_DATA_10 = 0x0e,
        ABSTRACT_DATA_11 = 0x0f,
        CONTROL_REGISTER = 0x10,
        STATUS_REGISTER = 0x11,
        ABSTRACT_CONTROL_STATUS_REGISTER = 0x16,
        ABSTRACT_COMMAND_REGISTER = 0x17,
    };
}
