#include "TargetDescriptor.hpp"

#include <utility>

#include "src/Exceptions/InternalFatalErrorException.hpp"

namespace Targets
{
    TargetDescriptor::TargetDescriptor(
        const std::string& name,
        TargetFamily family,
        const std::string& marketId,
        const std::string& vendorName,
        std::map<std::string, TargetAddressSpaceDescriptor>&& addressSpaceDescriptorsByKey,
        std::map<std::string, TargetPeripheralDescriptor>&& peripheralDescriptorsByKey,
        std::map<std::string, TargetPinoutDescriptor>&& pinoutDescriptorsByKey,
        std::vector<TargetVariantDescriptor>&& variantDescriptors,
        const BreakpointResources& breakpointResources
    )
        : name(name)
        , family(family)
        , marketId(marketId)
        , vendorName(vendorName)
        , addressSpaceDescriptorsByKey(std::move(addressSpaceDescriptorsByKey))
        , peripheralDescriptorsByKey(std::move(peripheralDescriptorsByKey))
        , pinoutDescriptorsByKey(std::move(pinoutDescriptorsByKey))
        , variantDescriptors(std::move(variantDescriptors))
        , breakpointResources(breakpointResources)
    {}

    std::optional<
        std::reference_wrapper<const TargetAddressSpaceDescriptor>
    > TargetDescriptor::tryGetAddressSpaceDescriptor(const std::string& key) const {
        const auto descriptorIt = this->addressSpaceDescriptorsByKey.find(key);

        if (descriptorIt == this->addressSpaceDescriptorsByKey.end()) {
            return std::nullopt;
        }

        return std::cref(descriptorIt->second);
    }

    const TargetAddressSpaceDescriptor& TargetDescriptor::getAddressSpaceDescriptor(const std::string& key) const {
        const auto descriptor = this->tryGetAddressSpaceDescriptor(key);
        if (!descriptor.has_value()) {
            throw Exceptions::InternalFatalErrorException{
                "Failed to get address space descriptor \"" + std::string{key}
                    + "\" from target descriptor - descriptor not found"
            };
        }

        return descriptor->get();
    }

    const TargetAddressSpaceDescriptor& TargetDescriptor::getFirstAddressSpaceDescriptorContainingMemorySegment(
        const std::string& memorySegmentKey
    ) const {
        for (const auto& [key, addressSpaceDescriptor] : this->addressSpaceDescriptorsByKey) {
            const auto segmentDescriptor = addressSpaceDescriptor.tryGetMemorySegmentDescriptor(memorySegmentKey);
            if (segmentDescriptor.has_value()) {
                return addressSpaceDescriptor;
            }
        }

        throw Exceptions::InternalFatalErrorException{
            "Failed to get address space descriptor from target descriptor - descriptor containing memory segment \""
                + memorySegmentKey + "\" not found"
        };
    }

    std::optional<
        std::reference_wrapper<const TargetPeripheralDescriptor>
    > TargetDescriptor::tryGetPeripheralDescriptor(const std::string& key) const {
        const auto descriptorIt = this->peripheralDescriptorsByKey.find(key);

        if (descriptorIt == this->peripheralDescriptorsByKey.end()) {
            return std::nullopt;
        }

        return std::cref(descriptorIt->second);
    }

    const TargetPeripheralDescriptor& TargetDescriptor::getPeripheralDescriptor(const std::string& key) const {
        const auto descriptor = this->tryGetPeripheralDescriptor(key);
        if (!descriptor.has_value()) {
            throw Exceptions::InternalFatalErrorException{
                "Failed to get peripheral descriptor \"" + std::string{key}
                    + "\" from target descriptor - descriptor not found"
            };
        }

        return descriptor->get();
    }

    std::optional<
        std::reference_wrapper<const TargetPinoutDescriptor>
    > TargetDescriptor::tryGetPinoutDescriptor(const std::string& key) const {
        const auto descriptorIt = this->pinoutDescriptorsByKey.find(key);

        if (descriptorIt == this->pinoutDescriptorsByKey.end()) {
            return std::nullopt;
        }

        return std::cref(descriptorIt->second);
    }

    const TargetPinoutDescriptor& TargetDescriptor::getPinoutDescriptor(const std::string& key) const {
        const auto descriptor = this->tryGetPinoutDescriptor(key);
        if (!descriptor.has_value()) {
            throw Exceptions::InternalFatalErrorException{
                "Failed to get pinout descriptor \"" + std::string{key}
                    + "\" from target descriptor - descriptor not found"
            };
        }

        return descriptor->get();
    }
}
