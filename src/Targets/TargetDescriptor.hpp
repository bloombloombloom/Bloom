#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <map>
#include <algorithm>
#include <QMetaType>

#include "TargetFamily.hpp"
#include "TargetMemory.hpp"
#include "TargetAddressSpaceDescriptor.hpp"
#include "TargetPeripheralDescriptor.hpp"
#include "TargetVariant.hpp"
#include "TargetBreakpoint.hpp"

namespace Targets
{
    struct TargetDescriptor
    {
        std::string name;
        TargetFamily family;
        std::string marketId;
        std::string vendorName;
        std::map<std::string, TargetAddressSpaceDescriptor> addressSpaceDescriptorsByKey;
        std::map<std::string, TargetPeripheralDescriptor> peripheralDescriptorsByKey;
        std::vector<TargetVariant> variants;
        BreakpointResources breakpointResources;

        TargetDescriptor(
            const std::string& name,
            TargetFamily family,
            const std::string& vendorName,
            const std::string& marketName,
            const std::map<std::string, TargetAddressSpaceDescriptor>& addressSpaceDescriptorsByKey,
            const std::map<std::string, TargetPeripheralDescriptor>& peripheralDescriptorsByKey,
            const std::vector<TargetVariant>& variants,
            const BreakpointResources& breakpointResources
        );

        std::optional<std::reference_wrapper<const TargetAddressSpaceDescriptor>> tryGetAddressSpaceDescriptor(
            const std::string& key
        ) const;

        const TargetAddressSpaceDescriptor& getAddressSpaceDescriptor(const std::string& key) const;
    };
}

Q_DECLARE_METATYPE(Targets::TargetDescriptor)
