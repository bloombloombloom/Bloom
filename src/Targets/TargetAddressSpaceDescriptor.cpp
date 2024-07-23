#include "TargetAddressSpaceDescriptor.hpp"

#include <utility>
#include <algorithm>

#include "src/Services/StringService.hpp"
#include "src/Exceptions/InternalFatalErrorException.hpp"

namespace Targets
{
    TargetAddressSpaceDescriptor::TargetAddressSpaceDescriptor(
        const std::string& key,
        const TargetMemoryAddressRange& addressRange,
        TargetMemoryEndianness endianness,
        std::map<std::string, TargetMemorySegmentDescriptor>&& segmentDescriptorsByKey,
        std::uint8_t unitSize
    )
        : id(TargetAddressSpaceDescriptor::generateId(key))
        , key(key)
        , addressRange(addressRange)
        , endianness(endianness)
        , segmentDescriptorsByKey(std::move(segmentDescriptorsByKey))
        , unitSize(unitSize)
    {}

    bool TargetAddressSpaceDescriptor::operator == (const TargetAddressSpaceDescriptor& other) const {
        return this->id == other.id;
    }

    bool TargetAddressSpaceDescriptor::operator != (const TargetAddressSpaceDescriptor& other) const {
        return !(*this == other);
    }

    TargetMemorySize TargetAddressSpaceDescriptor::size() const {
        return this->addressRange.size() * this->unitSize;
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
            throw Exceptions::InternalFatalErrorException{
                "Failed to get memory segment descriptor \"" + key + "\" from address space \"" + this->key
                    + "\" - segment not found"
            };
        }

        return segment->get();
    }

    std::vector<
        const TargetMemorySegmentDescriptor*
    > TargetAddressSpaceDescriptor::getIntersectingMemorySegmentDescriptors(
        const TargetMemoryAddressRange& addressRange
    ) const {
        auto output = std::vector<const TargetMemorySegmentDescriptor*>{};

        for (const auto& [key, segmentDescriptor] : this->segmentDescriptorsByKey) {
            if (segmentDescriptor.addressRange.intersectsWith(addressRange)) {
                output.push_back(&segmentDescriptor);
            }
        }

        std::sort(
            output.begin(),
            output.end(),
            [] (const TargetMemorySegmentDescriptor* descA, const TargetMemorySegmentDescriptor* descB) {
                return descA->addressRange.startAddress < descB->addressRange.startAddress;
            }
        );

        return output;
    }

    TargetAddressSpaceDescriptor TargetAddressSpaceDescriptor::clone() const {
        auto output = TargetAddressSpaceDescriptor{
            this->key,
            this->addressRange,
            this->endianness,
            {},
            this->unitSize
        };

        for (const auto& [key, descriptor] : this->segmentDescriptorsByKey) {
            output.segmentDescriptorsByKey.emplace(key, descriptor.clone());
        }

        return output;
    }

    TargetAddressSpaceId TargetAddressSpaceDescriptor::generateId(const std::string& addressSpaceKey) {
        return static_cast<TargetAddressSpaceId>(Services::StringService::hash(addressSpaceKey));
    }
}
