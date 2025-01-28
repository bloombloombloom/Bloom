#pragma once

#include <set>
#include <vector>

#include "TargetMemory.hpp"

namespace Targets
{
    struct TargetMemoryAddressRange
    {
        TargetMemoryAddress startAddress = 0;
        TargetMemoryAddress endAddress = 0;

        TargetMemoryAddressRange() = default;
        TargetMemoryAddressRange(TargetMemoryAddress startAddress, TargetMemoryAddress endAddress);

        bool operator == (const TargetMemoryAddressRange& rhs) const;
        bool operator < (const TargetMemoryAddressRange& rhs) const;

        /**
         * Returns the number of addresses in the range.
         *
         * Keep in mind that the number of addresses may not be equal to the number of bytes in the range. It depends
         * on whether the address space is byte-addressable. See TargetAddressSpaceDescriptor::unitSize for more.
         *
         * @return
         */
        [[nodiscard]] TargetMemorySize size() const;

        /**
         * Checks if this range intersects with the given range.
         *
         * @param other
         * @return
         */
        [[nodiscard]] bool intersectsWith(const TargetMemoryAddressRange& other) const noexcept;

        /**
         * Returns the number of addresses in this range that intersect with the given range.
         *
         * @param other
         * @return
         */
        [[nodiscard]] TargetMemorySize intersectingSize(const TargetMemoryAddressRange& other) const noexcept;

        /**
         * Checks if the given address is contained within this range.
         *
         * @param address
         * @return
         */
        [[nodiscard]] bool contains(TargetMemoryAddress address) const noexcept;

        /**
         * Checks if the given range is completely contained within this range.
         *
         * @param addressRange
         * @return
         */
        [[nodiscard]] bool contains(const TargetMemoryAddressRange& addressRange) const noexcept;

        /**
         * Returns a set of all addresses within this range.
         *
         * @return
         */
        [[nodiscard]] std::set<TargetMemoryAddress> addresses() const noexcept;

        /**
         * Splits the address range into a vector of blocks, with the given block size.
         *
         * @param blockSize
         * @return
         */
        [[nodiscard]] std::vector<TargetMemoryAddressRange> blocks(TargetMemorySize blockSize) const noexcept;
    };
}
