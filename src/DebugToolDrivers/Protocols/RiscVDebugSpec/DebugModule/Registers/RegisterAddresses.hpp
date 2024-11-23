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
        ABSTRACT_DATA_6 = 0x0A,
        ABSTRACT_DATA_7 = 0x0B,
        ABSTRACT_DATA_8 = 0x0C,
        ABSTRACT_DATA_9 = 0x0D,
        ABSTRACT_DATA_10 = 0x0E,
        ABSTRACT_DATA_11 = 0x0F,
        CONTROL_REGISTER = 0x10,
        STATUS_REGISTER = 0x11,
        ABSTRACT_CONTROL_STATUS_REGISTER = 0x16,
        ABSTRACT_COMMAND_REGISTER = 0x17,
        ABSTRACT_COMMAND_AUTO_EXECUTE_REGISTER = 0x18,
        PROGRAM_BUFFER_0 = 0x20,
        PROGRAM_BUFFER_1 = 0x21,
        PROGRAM_BUFFER_2 = 0x22,
        PROGRAM_BUFFER_3 = 0x23,
        PROGRAM_BUFFER_4 = 0x24,
        PROGRAM_BUFFER_5 = 0x25,
        PROGRAM_BUFFER_6 = 0x26,
        PROGRAM_BUFFER_7 = 0x27,
        PROGRAM_BUFFER_8 = 0x28,
        PROGRAM_BUFFER_9 = 0x29,
        PROGRAM_BUFFER_10 = 0x2A,
        PROGRAM_BUFFER_11 = 0x2B,
        PROGRAM_BUFFER_12 = 0x2C,
        PROGRAM_BUFFER_13 = 0x2D,
        PROGRAM_BUFFER_14 = 0x2E,
        PROGRAM_BUFFER_15 = 0x2F,
    };
}
