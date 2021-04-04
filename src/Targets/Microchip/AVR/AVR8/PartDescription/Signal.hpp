#pragma once

#include <string>
#include <optional>

namespace Bloom::Targets::Microchip::Avr::Avr8Bit::PartDescription
{
    struct Signal {
        std::string padName;
        std::string function;
        std::optional<int> index;
        std::string group;
    };
}
