#pragma once

#include <map>
#include <vector>
#include <string>

#include "RegisterGroup.hpp"
#include "Signal.hpp"

namespace Targets::TargetDescription
{
    struct ModuleInstance
    {
        std::string name;
        std::map<std::string, RegisterGroup> registerGroupsMappedByName;
        std::vector<Signal> instanceSignals;
    };
}
