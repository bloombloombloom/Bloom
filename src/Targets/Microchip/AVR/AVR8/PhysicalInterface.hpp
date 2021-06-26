#pragma once

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    enum class PhysicalInterface: int
    {
        JTAG,
        DEBUG_WIRE,
        PDI,
        UPDI,
    };
}
