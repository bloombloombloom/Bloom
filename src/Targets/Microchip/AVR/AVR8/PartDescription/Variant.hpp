#pragma once

#include <string>

namespace Bloom::Targets::Microchip::Avr::Avr8Bit::PartDescription
{
    struct Variant {
        std::string orderCode;
        std::string pinoutName;
        std::string package;
        bool disabled = false;
    };
}
