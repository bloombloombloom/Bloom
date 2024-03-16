#pragma once

#include <cstdint>
#include <atomic>
#include <string>
#include <map>
#include <optional>

#include "TargetMemory.hpp"
#include "TargetMemorySegmentDescriptor.hpp"
#include "src/Exceptions/InternalFatalErrorException.hpp"

namespace Targets
{
    using TargetAddressSpaceDescriptorId = std::uint8_t;

    struct TargetAddressSpaceDescriptor
    {
    public:
        const TargetAddressSpaceDescriptorId id;
        std::string key;
        TargetMemoryAddress startAddress;
        TargetMemorySize size;
        TargetMemoryEndianness endianness;
        std::map<std::string, TargetMemorySegmentDescriptor> segmentDescriptorsByKey;

        TargetAddressSpaceDescriptor(
            const std::string& key,
            TargetMemoryAddress startAddress,
            TargetMemorySize size,
            TargetMemoryEndianness endianness,
            const std::map<std::string, TargetMemorySegmentDescriptor>& segmentDescriptorsByKey
        )
            : id(++(TargetAddressSpaceDescriptor::lastAddressSpaceDescriptorId))
            , key(key)
            , startAddress(startAddress)
            , size(size)
            , endianness(endianness)
            , segmentDescriptorsByKey(segmentDescriptorsByKey)
        {};

        std::optional<std::reference_wrapper<const TargetMemorySegmentDescriptor>> tryGetMemorySegmentDescriptor(
            const std::string& key
        ) const {
            const auto segmentIt = this->segmentDescriptorsByKey.find(key);

            if (segmentIt == this->segmentDescriptorsByKey.end()) {
                return std::nullopt;
            }

            return std::cref(segmentIt->second);
        }

        const TargetMemorySegmentDescriptor& getMemorySegmentDescriptor(const std::string& key) const {
            const auto segment = this->tryGetMemorySegmentDescriptor(key);
            if (!segment.has_value()) {
                throw Exceptions::InternalFatalErrorException(
                    "Failed to get memory segment descriptor \"" + key + "\" from address space \"" + this->key
                        + "\" - segment not found"
                );
            }

            return segment->get();
        }

    private:
        static inline std::atomic<TargetAddressSpaceDescriptorId> lastAddressSpaceDescriptorId = 0;
    };
}
