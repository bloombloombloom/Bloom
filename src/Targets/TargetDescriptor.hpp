#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <map>
#include <QMetaType>

#include "TargetMemory.hpp"
#include "TargetRegister.hpp"
#include "TargetVariant.hpp"

namespace Bloom::Targets
{
    struct TargetDescriptor
    {
        std::string name;
        std::string id;
        std::map<TargetMemoryType, TargetMemoryDescriptor> memoryDescriptorsByType;
        std::map<TargetRegisterType, TargetRegisterDescriptors> registerDescriptorsByType;
        std::vector<TargetVariant> variants;

        TargetMemoryType programMemoryType;
    };
}

Q_DECLARE_METATYPE(Bloom::Targets::TargetDescriptor)
