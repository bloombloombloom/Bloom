#pragma once

#include <cstdint>

namespace TargetController::Commands
{
    enum class CommandType: std::uint8_t
    {
        GENERIC,
        SHUTDOWN,
        GET_TARGET_DESCRIPTOR,
        START_ATOMIC_SESSION,
        END_ATOMIC_SESSION,
        STOP_TARGET_EXECUTION,
        RESUME_TARGET_EXECUTION,
        RESET_TARGET,
        READ_TARGET_REGISTERS,
        WRITE_TARGET_REGISTERS,
        READ_TARGET_MEMORY,
        WRITE_TARGET_MEMORY,
        ERASE_TARGET_MEMORY,
        GET_TARGET_STATE,
        STEP_TARGET_EXECUTION,
        SET_BREAKPOINT_ANY_TYPE,
        REMOVE_PROGRAM_BREAKPOINT,
        SET_TARGET_PROGRAM_COUNTER,
        SET_TARGET_STACK_POINTER,
        GET_TARGET_GPIO_PAD_STATES,
        SET_TARGET_GPIO_PAD_STATE,
        GET_TARGET_STACK_POINTER,
        GET_TARGET_PROGRAM_COUNTER,
        ENABLE_PROGRAMMING_MODE,
        DISABLE_PROGRAMMING_MODE,
        INVOKE_TARGET_PASSTHROUGH_COMMAND,
    };
}
