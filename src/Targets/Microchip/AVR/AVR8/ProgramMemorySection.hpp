#pragma once

#include <cstdint>

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    enum class ProgramMemorySection: std::uint8_t
    {
        APPLICATION,
        BOOT,
    };
}
