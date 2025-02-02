#pragma once

#include <cstdint>

namespace DebugToolDrivers::Protocols::RiscVDebug::TriggerModule
{
    using RegisterValue = std::uint32_t;
    using TriggerIndex = std::uint32_t;

    enum class TriggerType: std::uint8_t
    {
        NONE = 0x00,
        LEGACY = 0x01,
        MATCH_CONTROL = 0x02,
        INSTRUCTION_COUNT = 0x03,
        INTERRUPT_TRIGGER = 0x04,
        EXCEPTION_TRIGGER = 0x05,
        MATCH_CONTROL_TYPE_6 = 0x06,
        EXTERNAL = 0x07,
        DISABLED = 0x0F,
    };

    enum class TriggerAction: std::uint8_t
    {
        RAISE_BREAKPOINT_EXCEPTION = 0x00,
        ENTER_DEBUG_MODE = 0x01,
    };
}
