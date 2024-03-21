#include "TargetDescriptor.hpp"

#include "src/Exceptions/InternalFatalErrorException.hpp"

namespace Targets
{
    TargetDescriptor::TargetDescriptor(
        const std::string& name,
        TargetFamily family,
        const std::string& marketId,
        const std::string& vendorName,
        const std::map<std::string, TargetAddressSpaceDescriptor>& addressSpaceDescriptorsByKey,
        const std::map<std::string, TargetPeripheralDescriptor>& peripheralDescriptorsByKey,
        const std::vector<TargetVariant>& variants,
        const BreakpointResources& breakpointResources
    )
        : name(name)
        , family(family)
        , marketId(marketId)
        , vendorName(vendorName)
        , addressSpaceDescriptorsByKey(addressSpaceDescriptorsByKey)
        , peripheralDescriptorsByKey(peripheralDescriptorsByKey)
        , variants(variants)
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
            throw Exceptions::InternalFatalErrorException(
                "Failed to get address space descriptor \"" + std::string(key)
                    + "\" from target descriptor - descriptor not found"
            );
        }

        return descriptor->get();
    }
}
