#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <map>

#include "TargetRegister.hpp"
#include "TargetVariant.hpp"

namespace Bloom::Targets
{
    struct TargetDescriptor
    {
        std::string name;
        std::string id;
        std::uint32_t ramSize;
        std::map<TargetRegisterType, TargetRegisterDescriptors> registerDescriptorsByType;
        std::vector<TargetVariant> variants;
    };
}

Q_DECLARE_METATYPE(Bloom::Targets::TargetDescriptor)
