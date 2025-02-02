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
        std::map<std::string, TargetPadDescriptor>&& padDescriptorsByKey,
        std::map<std::string, TargetPinoutDescriptor>&& pinoutDescriptorsByKey,
        std::map<std::string, TargetVariantDescriptor>&& variantDescriptorsByKey,
        const BreakpointResources& breakpointResources
    )
        : name(name)
        , family(family)
        , marketId(marketId)
        , vendorName(vendorName)
        , addressSpaceDescriptorsByKey(std::move(addressSpaceDescriptorsByKey))
        , peripheralDescriptorsByKey(std::move(peripheralDescriptorsByKey))
        , padDescriptorsByKey(std::move(padDescriptorsByKey))
        , pinoutDescriptorsByKey(std::move(pinoutDescriptorsByKey))
        , variantDescriptorsByKey(std::move(variantDescriptorsByKey))
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

    std::optional<
        std::reference_wrapper<TargetAddressSpaceDescriptor>
    > TargetDescriptor::tryGetAddressSpaceDescriptor(const std::string& key) {
        const auto descriptorIt = this->addressSpaceDescriptorsByKey.find(key);

        if (descriptorIt == this->addressSpaceDescriptorsByKey.end()) {
            return std::nullopt;
        }

        return std::ref(descriptorIt->second);
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

    TargetAddressSpaceDescriptor& TargetDescriptor::getAddressSpaceDescriptor(const std::string& key) {
        return const_cast<TargetAddressSpaceDescriptor&>(
            const_cast<const TargetDescriptor*>(this)->getAddressSpaceDescriptor(key)
        );
    }

    const TargetMemorySegmentDescriptor& TargetDescriptor::getMemorySegmentDescriptor(
        const std::string& addressSpaceKey,
        const std::string& segmentKey
    ) const {
        return this->getAddressSpaceDescriptor(addressSpaceKey).getMemorySegmentDescriptor(segmentKey);
    }

    TargetMemorySegmentDescriptor& TargetDescriptor::getMemorySegmentDescriptor(
        const std::string& addressSpaceKey,
        const std::string& segmentKey
    ) {
        return const_cast<TargetMemorySegmentDescriptor&>(
            const_cast<const TargetDescriptor*>(this)->getMemorySegmentDescriptor(addressSpaceKey, segmentKey)
        );
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
            "Failed to get address space descriptor from target descriptor - descriptor containing memory segment `"
                + memorySegmentKey + "` not found"
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

    TargetPeripheralDescriptor& TargetDescriptor::getPeripheralDescriptor(const std::string& key) {
        return const_cast<TargetPeripheralDescriptor&>(
            const_cast<const TargetDescriptor*>(this)->getPeripheralDescriptor(key)
        );
    }

    std::optional<
        std::reference_wrapper<const TargetPadDescriptor>
    > TargetDescriptor::tryGetPadDescriptor(const std::string& key) const {
        const auto descriptorIt = this->padDescriptorsByKey.find(key);

        if (descriptorIt == this->padDescriptorsByKey.end()) {
            return std::nullopt;
        }

        return std::cref(descriptorIt->second);
    }

    const TargetPadDescriptor& TargetDescriptor::getPadDescriptor(const std::string& key) const {
        const auto descriptor = this->tryGetPadDescriptor(key);
        if (!descriptor.has_value()) {
            throw Exceptions::InternalFatalErrorException{
                "Failed to get pad descriptor \"" + std::string{key}
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

    std::optional<
        std::reference_wrapper<const TargetVariantDescriptor>
    > TargetDescriptor::tryGetVariantDescriptor(const std::string& key) const {
        const auto descriptorIt = this->variantDescriptorsByKey.find(key);

        if (descriptorIt == this->variantDescriptorsByKey.end()) {
            return std::nullopt;
        }

        return std::cref(descriptorIt->second);
    }

    const TargetVariantDescriptor& TargetDescriptor::getVariantDescriptor(const std::string& key) const {
        const auto descriptor = this->tryGetVariantDescriptor(key);
        if (!descriptor.has_value()) {
            throw Exceptions::InternalFatalErrorException{
                "Failed to get pinout descriptor \"" + std::string{key}
                    + "\" from target descriptor - descriptor not found"
            };
        }

        return descriptor->get();
    }
}
