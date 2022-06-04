#pragma once

namespace Bloom::Targets::Microchip::Avr::Avr8Bit
{
    struct ProgrammingSession
    {
        bool chipErased = false;
        bool applicationSectionErased = false;
        bool bootSectionErased = false;
    };
}
