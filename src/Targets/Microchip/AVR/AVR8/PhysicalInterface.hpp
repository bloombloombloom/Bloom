#pragma once

#include <cstdint>

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    enum class PhysicalInterface: std::uint8_t
    {
        JTAG,
        DEBUG_WIRE,
        PDI,
        UPDI,
    };
}
