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
#include "TargetBreakpoint.hpp"

namespace Targets
{
    enum class TargetFamily: std::uint8_t
    {
        AVR8,
        RISC_V,
    };

    struct TargetDescriptor
    {
        std::string id;
        TargetFamily family;
        std::string name;
        std::string vendorName;
        std::map<TargetMemoryType, TargetMemoryDescriptor> memoryDescriptorsByType;
        std::map<TargetRegisterDescriptorId, TargetRegisterDescriptor> registerDescriptorsById;
        BreakpointResources breakpointResources;
        std::vector<TargetVariant> variants;

        TargetMemoryType programMemoryType;

        TargetDescriptor(
            const std::string& id,
            TargetFamily family,
            const std::string& name,
            const std::string& vendorName,
            const std::map<TargetMemoryType, TargetMemoryDescriptor>& memoryDescriptorsByType,
            const std::map<TargetRegisterDescriptorId, TargetRegisterDescriptor>& registerDescriptorsById,
            const BreakpointResources& breakpointResources,
            const std::vector<TargetVariant>& variants,
            TargetMemoryType programMemoryType
        )
            : id(id)
            , family(family)
            , name(name)
            , vendorName(vendorName)
            , memoryDescriptorsByType(memoryDescriptorsByType)
            , registerDescriptorsById(registerDescriptorsById)
            , breakpointResources(breakpointResources)
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
