#pragma once

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    struct ProgrammingSession
    {
        bool applicationSectionErased = false;
        bool bootSectionErased = false;
    };
}
