#pragma once

#include <cstdint>

namespace Bloom::TargetController::Commands
{
    enum class CommandType: std::uint8_t
    {
        GENERIC,
        GET_TARGET_CONTROLLER_STATE,
        STOP_TARGET_EXECUTION,
        RESUME_TARGET_EXECUTION,
        RESET_TARGET,
        READ_TARGET_REGISTERS,
        WRITE_TARGET_REGISTERS,
        READ_TARGET_MEMORY,
    };
}
