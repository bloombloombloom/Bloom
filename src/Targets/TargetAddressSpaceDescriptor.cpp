#include "TargetAddressSpaceDescriptor.hpp"

#include "src/Exceptions/InternalFatalErrorException.hpp"

namespace Targets
{
    TargetAddressSpaceDescriptor::TargetAddressSpaceDescriptor(
        const std::string& key,
        const TargetMemoryAddressRange& addressRange,
        TargetMemoryEndianness endianness,
        const std::map<std::string, TargetMemorySegmentDescriptor>& segmentDescriptorsByKey
    )
        : id(++(TargetAddressSpaceDescriptor::lastAddressSpaceDescriptorId))
        , key(key)
        , addressRange(addressRange)
        , endianness(endianness)
        , segmentDescriptorsByKey(segmentDescriptorsByKey)
    {}

    TargetMemorySize TargetAddressSpaceDescriptor::size() const {
        return this->addressRange.size();
    }

    std::optional<
        std::reference_wrapper<const TargetMemorySegmentDescriptor>
    > TargetAddressSpaceDescriptor::tryGetMemorySegmentDescriptor(const std::string& key) const {
        const auto segmentIt = this->segmentDescriptorsByKey.find(key);

        if (segmentIt == this->segmentDescriptorsByKey.end()) {
            return std::nullopt;
        }

        return std::cref(segmentIt->second);
    }

    const TargetMemorySegmentDescriptor& TargetAddressSpaceDescriptor::getMemorySegmentDescriptor(
        const std::string& key
    ) const {
        const auto segment = this->tryGetMemorySegmentDescriptor(key);
        if (!segment.has_value()) {
            throw Exceptions::InternalFatalErrorException(
                "Failed to get memory segment descriptor \"" + key + "\" from address space \"" + this->key
                    + "\" - segment not found"
            );
        }

        return segment->get();
    }
}
