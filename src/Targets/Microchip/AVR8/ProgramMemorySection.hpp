#pragma once

#include <cstdint>

namespace Targets::Microchip::Avr8
{
    enum class ProgramMemorySection: std::uint8_t
    {
        APPLICATION,
        BOOT,
    };
}
