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
    struct TargetAddressSpaceDescriptor
    {
    public:
        /*
         * The ID of an address space is just a hash of the address space key, which is unique. We use the ID for
         * cheap equality checks.
         *
         * See TargetAddressSpaceDescriptor::generateId() for more.
         */
        const TargetAddressSpaceId id;
        const std::string key;
        TargetMemoryAddressRange addressRange;
        TargetMemoryEndianness endianness;
        std::map<std::string, TargetMemorySegmentDescriptor> segmentDescriptorsByKey;

        /*
         * In Bloom, a byte is always considered to be 8 bits in width.
         *
         * Not all address spaces are byte-addressable. TargetAddressSpaceDescriptor::unitSize holds the number of
         * bytes within a single addressable unit.
         *
         * This is also available in segment descriptors, via TargetMemorySegmentDescriptor::addressSpaceUnitSize.
         */
        std::uint8_t unitSize;

        TargetAddressSpaceDescriptor(
            const std::string& key,
            const TargetMemoryAddressRange& addressRange,
            TargetMemoryEndianness endianness,
            std::map<std::string, TargetMemorySegmentDescriptor>&& segmentDescriptorsByKey,
            std::uint8_t unitSize = 1
        );

        TargetAddressSpaceDescriptor(const TargetAddressSpaceDescriptor& other) = delete;
        TargetAddressSpaceDescriptor& operator = (const TargetAddressSpaceDescriptor& other) = delete;

        TargetAddressSpaceDescriptor(TargetAddressSpaceDescriptor&& other) noexcept = default;

        bool operator == (const TargetAddressSpaceDescriptor& other) const;
        bool operator != (const TargetAddressSpaceDescriptor& other) const;

        [[nodiscard]] TargetMemorySize size() const;

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

        /**
         * Fetches the memory segment which contains the given address.
         *
         * @param address
         *
         * @return
         *   The containing memory segment descriptor, or std::nullopt of no segments contain the address.
         */
        [[nodiscard]] std::optional<
            std::reference_wrapper<const TargetMemorySegmentDescriptor>
        > getContainingMemorySegmentDescriptor(const TargetMemoryAddress& address) const;

        [[nodiscard]] TargetAddressSpaceDescriptor clone() const;

        static TargetAddressSpaceId generateId(const std::string& addressSpaceKey);
    };
}
