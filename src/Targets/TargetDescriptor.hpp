#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <map>
#include <algorithm>
#include <QMetaType>

#include "TargetMemory.hpp"
#include "TargetRegister.hpp"
#include "TargetVariant.hpp"

namespace Targets
{
    struct TargetDescriptor
    {
        std::string name;
        std::string id;
        std::string vendorName;
        std::map<TargetMemoryType, TargetMemoryDescriptor> memoryDescriptorsByType;
        std::map<TargetRegisterDescriptorId, TargetRegisterDescriptor> registerDescriptorsById;
        std::vector<TargetVariant> variants;

        TargetMemoryType programMemoryType;

        TargetDescriptor(
            const std::string& id,
            const std::string& name,
            const std::string& vendorName,
            const std::map<TargetMemoryType, TargetMemoryDescriptor>& memoryDescriptorsByType,
            const std::map<TargetRegisterDescriptorId, TargetRegisterDescriptor>& registerDescriptorsById,
            const std::vector<TargetVariant>& variants,
            TargetMemoryType programMemoryType
        )
            : name(name)
            , id(id)
            , vendorName(vendorName)
            , memoryDescriptorsByType(memoryDescriptorsByType)
            , registerDescriptorsById(registerDescriptorsById)
            , variants(variants)
            , programMemoryType(programMemoryType)
        {}

        TargetRegisterDescriptorIds registerDescriptorIdsForType(TargetRegisterType type) {
            auto output = TargetRegisterDescriptorIds();

            for (const auto& [descriptorId, descriptor] : this->registerDescriptorsById) {
                if (descriptor.type == type) {
                    output.insert(descriptorId);
                }
            }

            return output;
        }
    };
}

Q_DECLARE_METATYPE(Targets::TargetDescriptor)
