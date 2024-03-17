#pragma once

#include <cstdint>
#include <atomic>
#include <string>
#include <map>
#include <optional>

#include "TargetMemory.hpp"
#include "TargetMemorySegmentDescriptor.hpp"

namespace Targets
{
    using TargetAddressSpaceDescriptorId = std::uint8_t;

    struct TargetAddressSpaceDescriptor
    {
    public:
        const TargetAddressSpaceDescriptorId id;
        std::string key;
        TargetMemoryAddressRange addressRange;
        TargetMemoryEndianness endianness;
        std::map<std::string, TargetMemorySegmentDescriptor> segmentDescriptorsByKey;

        TargetAddressSpaceDescriptor(
            const std::string& key,
            const TargetMemoryAddressRange& addressRange,
            TargetMemoryEndianness endianness,
            const std::map<std::string, TargetMemorySegmentDescriptor>& segmentDescriptorsByKey
        );

        TargetMemorySize size() const;

        /**
         * Attempts to fetch a memory segment descriptor with the given key.
         *
         * @param key
         *  The key of the memory segment descriptor to lookup.
         *
         * @return
         *  A reference wrapper of the memory segment descriptor, if found. Otherwise, std::nullopt.
         */
        std::optional<std::reference_wrapper<const TargetMemorySegmentDescriptor>> tryGetMemorySegmentDescriptor(
            const std::string& key
        ) const;

        /**
         * Fetches a memory segment descriptor with the given key. If the descriptor doesn't exist, an
         * InternalFatalErrorException is thrown.
         *
         * @param key
         *  The key of the memory segment descriptor to lookup.
         *
         * @return
         *  A reference to the memory segment descriptor.
         */
        const TargetMemorySegmentDescriptor& getMemorySegmentDescriptor(const std::string& key) const;

        /**
         * Fetches all memory segments in the address space that intersect with the given address range.
         *
         * @param addressRange
         *  The address range with which the memory segments must intersect.
         *
         * @return
         *  Pointers to descriptors of all intersecting memory segments.
         */
        std::vector<const TargetMemorySegmentDescriptor*> getIntersectingMemorySegmentDescriptors(
            const TargetMemoryAddressRange& addressRange
        ) const;

    private:
        static inline std::atomic<TargetAddressSpaceDescriptorId> lastAddressSpaceDescriptorId = 0;
    };
}
