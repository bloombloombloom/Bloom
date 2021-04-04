#pragma once

#include <string>
#include <vector>

namespace Bloom::Targets::Microchip::Avr::Avr8Bit::PartDescription
{
    struct Pin {
        std::string pad;
        int position;
    };

    struct Pinout {
        std::string name;
        std::vector<Pin> pins;
    };
}
