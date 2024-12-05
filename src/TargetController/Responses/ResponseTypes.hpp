#pragma once

#include <cstdint>

namespace TargetController::Responses
{
    enum class ResponseType: std::uint8_t
    {
        GENERIC,
        ERROR,
        ATOMIC_SESSION_ID,
        TARGET_DESCRIPTOR,
        TARGET_REGISTERS_READ,
        TARGET_MEMORY_READ,
        TARGET_STATE,
        TARGET_GPIO_PAD_STATES,
        TARGET_STACK_POINTER,
        TARGET_PROGRAM_COUNTER,
        PROGRAM_BREAKPOINT,
    };
}
