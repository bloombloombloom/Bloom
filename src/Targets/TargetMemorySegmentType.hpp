#pragma once

#include <cstdint>

namespace Targets
{
    enum class TargetMemorySegmentType: std::uint8_t
    {
        ALIASED,
        REGISTERS,
        EEPROM,
        FLASH,
        FUSES,
        IO,
        RAM,
        LOCKBITS,
        OSCCAL,
        PRODUCTION_SIGNATURES,
        SIGNATURES,
        USER_SIGNATURES,
    };
}
