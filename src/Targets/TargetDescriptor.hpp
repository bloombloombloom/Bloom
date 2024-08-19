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
#include "TargetPadDescriptor.hpp"
#include "TargetPinoutDescriptor.hpp"
#include "TargetVariantDescriptor.hpp"
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
        std::map<std::string, TargetPadDescriptor> padDescriptorsByKey;
        std::map<std::string, TargetPinoutDescriptor> pinoutDescriptorsByKey;
        std::map<std::string, TargetVariantDescriptor> variantDescriptorsByKey;
        BreakpointResources breakpointResources;

        TargetDescriptor(
            const std::string& name,
            TargetFamily family,
            const std::string& marketId,
            const std::string& vendorName,
            std::map<std::string, TargetAddressSpaceDescriptor>&& addressSpaceDescriptorsByKey,
            std::map<std::string, TargetPeripheralDescriptor>&& peripheralDescriptorsByKey,
            std::map<std::string, TargetPadDescriptor>&& padDescriptorsByKey,
            std::map<std::string, TargetPinoutDescriptor>&& pinoutDescriptorsByKey,
            std::map<std::string, TargetVariantDescriptor>&& variantDescriptorsByKey,
            const BreakpointResources& breakpointResources
        );

        TargetDescriptor(const TargetDescriptor& other) = delete;
        TargetDescriptor& operator = (const TargetDescriptor& other) = delete;

        TargetDescriptor(TargetDescriptor&& other) noexcept = default;
        TargetDescriptor& operator = (TargetDescriptor&& other) = default;

        std::optional<std::reference_wrapper<const TargetAddressSpaceDescriptor>> tryGetAddressSpaceDescriptor(
            const std::string& key
        ) const;

        const TargetAddressSpaceDescriptor& getAddressSpaceDescriptor(const std::string& key) const;

        /**
         * Returns the descriptor for the first address space that contains the given memory segment.
         *
         * @param memorySegmentKey
         *  Key of the memory segment that should be contained within the address space.
         *
         * @return
         */
        const TargetAddressSpaceDescriptor& getFirstAddressSpaceDescriptorContainingMemorySegment(
            const std::string& memorySegmentKey
        ) const;

        std::optional<std::reference_wrapper<const TargetPeripheralDescriptor>> tryGetPeripheralDescriptor(
            const std::string& key
        ) const;

        const TargetPeripheralDescriptor& getPeripheralDescriptor(const std::string& key) const;

        std::optional<std::reference_wrapper<const TargetPadDescriptor>> tryGetPadDescriptor(
                const std::string& key
        ) const;

        const TargetPadDescriptor& getPadDescriptor(const std::string& key) const;

        std::optional<std::reference_wrapper<const TargetPinoutDescriptor>> tryGetPinoutDescriptor(
            const std::string& key
        ) const;

        const TargetPinoutDescriptor& getPinoutDescriptor(const std::string& key) const;

        std::optional<std::reference_wrapper<const TargetVariantDescriptor>> tryGetVariantDescriptor(
            const std::string& key
        ) const;

        const TargetVariantDescriptor& getVariantDescriptor(const std::string& key) const;
    };
}
