#pragma once

#include <cstdint>
#include <algorithm>
#include <vector>
#include <span>
#include <set>
#include <optional>
#include <cassert>

namespace Targets
{
    using TargetMemoryAddress = std::uint32_t;
    using TargetMemorySize = std::uint32_t;
    using TargetStackPointer = TargetMemoryAddress;
    using TargetMemoryBuffer = std::vector<unsigned char>;
    using TargetMemoryBufferSpan = std::span<const TargetMemoryBuffer::value_type>;

    using TargetAddressSpaceId = std::size_t;
    using TargetMemorySegmentId = std::size_t;

    enum class TargetMemoryEndianness: std::uint8_t
    {
        BIG,
        LITTLE,
    };

    struct TargetMemoryAddressRange
    {
        TargetMemoryAddress startAddress = 0;
        TargetMemoryAddress endAddress = 0;

        TargetMemoryAddressRange() = default;
        TargetMemoryAddressRange(TargetMemoryAddress startAddress, TargetMemoryAddress endAddress)
            : startAddress(startAddress)
            , endAddress(endAddress)
        {
            assert(this->startAddress <= this->endAddress);
        }

        bool operator == (const TargetMemoryAddressRange& rhs) const {
            return this->startAddress == rhs.startAddress && this->endAddress == rhs.endAddress;
        }

        bool operator < (const TargetMemoryAddressRange& rhs) const {
            return this->startAddress < rhs.startAddress;
        }

        /**
         * Returns the number of addresses in the range.
         *
         * Keep in mind that the number of addresses may not be equal to the number of bytes in the range. It depends
         * on whether the address space is byte-addressable. See TargetAddressSpaceDescriptor::unitSize for more.
         *
         * @return
         */
        [[nodiscard]] TargetMemorySize size() const {
            return this->endAddress - this->startAddress + 1;
        }

        /**
         * Checks if this range intersects with the given range.
         *
         * @param other
         * @return
         */
        [[nodiscard]] bool intersectsWith(const TargetMemoryAddressRange& other) const noexcept {
            return this->startAddress <= other.endAddress && other.startAddress <= this->endAddress;
        }

        /**
         * Returns the number of addresses in this range that intersect with the given range.
         *
         * @param other
         * @return
         */
        [[nodiscard]] TargetMemorySize intersectingSize(const TargetMemoryAddressRange& other) const noexcept {
            return this->intersectsWith(other)
                ? std::min(this->endAddress, other.endAddress) - std::max(this->startAddress, other.startAddress) + 1
                : 0;
        }

        /**
         * Checks if the given address is contained within this range.
         *
         * @param address
         * @return
         */
        [[nodiscard]] bool contains(TargetMemoryAddress address) const noexcept {
            return address >= this->startAddress && address <= this->endAddress;
        }

        /**
         * Checks if the given range is completely contained within this range.
         *
         * @param addressRange
         * @return
         */
        [[nodiscard]] bool contains(const TargetMemoryAddressRange& addressRange) const noexcept {
            return this->startAddress <= addressRange.startAddress && this->endAddress >= addressRange.endAddress;
        }

        /**
         * Returns a set of all addresses within this range.
         *
         * @return
         */
        std::set<Targets::TargetMemoryAddress> addresses() const noexcept {
            auto addresses = std::set<Targets::TargetMemoryAddress>{};
            auto addressesIt = addresses.end();

            for (auto i = this->startAddress; i <= this->endAddress; ++i) {
                addressesIt = addresses.insert(addressesIt, i);
            }

            return addresses;
        }
    };

    struct TargetMemoryAccess
    {
        bool readable = false;
        bool writeable = false;

        TargetMemoryAccess(
            bool readable,
            bool writeable
        )
            : readable(readable)
            , writeable(writeable)
        {}
    };
}
