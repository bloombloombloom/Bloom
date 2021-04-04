#pragma once

#include <map>
#include <vector>

#include "RegisterGroup.hpp"
#include "Signal.hpp"

namespace Bloom::Targets::Microchip::Avr::Avr8Bit::PartDescription
{
    struct ModuleInstance {
        std::string name;
        std::map<std::string, RegisterGroup> registerGroupsMappedByName;
        std::vector<Signal> instanceSignals;
    };
}
