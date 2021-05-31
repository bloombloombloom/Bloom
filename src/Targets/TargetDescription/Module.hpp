#pragma once

#include "ModuleInstance.hpp"
#include "RegisterGroup.hpp"

namespace Bloom::Targets::TargetDescription
{
    struct Module
    {
        std::string name;
        std::map<std::string, ModuleInstance> instancesMappedByName;
        std::map<std::string, RegisterGroup> registerGroupsMappedByName;
    };
}
