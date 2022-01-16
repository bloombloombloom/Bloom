#pragma once

#include <cstdint>
#include <atomic>
#include <QString>
#include <utility>

#include "src/Targets/TargetMemory.hpp"
#include "src/Helpers/DateTime.hpp"

namespace Bloom
{
    enum class MemoryRegionType: std::uint8_t
    {
        FOCUSED,
        EXCLUDED,
    };

    enum class MemoryRegionAddressType: std::uint8_t
    {
        ABSOLUTE,
        RELATIVE,
    };

    class MemoryRegion
    {
    public:
        std::size_t id = MemoryRegion::lastId++;
        QString name;
        QDateTime createdDate = DateTime::currentDateTime();
        MemoryRegionType type;

        /**
         * The addressRangeType holds the user's preference when inputting address ranges in the memory region manager
         * window, for this particular memory region.
         *
         * It has no significance anywhere else. We never store the address in relative form - this->addressRange will
         * always be in absolute form. Conversion is done at the point of applying the changes.
         *
         * For more, see the following and their call sites:
         * See RegionItem::convertRelativeToAbsoluteAddressRange()
         * See RegionItem::convertAbsoluteToRelativeAddressRange()
         */
        MemoryRegionAddressType addressRangeType = MemoryRegionAddressType::ABSOLUTE;

        /**
         * This address range will always be in absolute form. Regardless of the value of this->addressRangeType.
         * See the comment above, for this->addressRangeType, for more.
         */
        Targets::TargetMemoryAddressRange addressRange;

        MemoryRegion(
            QString name,
            MemoryRegionType type,
            const Targets::TargetMemoryAddressRange& addressRange
        ): name(std::move(name)), type(type), addressRange(addressRange) {};

        virtual ~MemoryRegion() = default;

        MemoryRegion(const MemoryRegion& other) = default;
        MemoryRegion(MemoryRegion&& other) = default;

        MemoryRegion& operator = (const MemoryRegion& other) = default;
        MemoryRegion& operator = (MemoryRegion&& other) = default;

        bool operator == (const MemoryRegion& other) const {
            return this->id == other.id;
        }

        bool operator != (const MemoryRegion& other) const {
            return !(*this == other);
        }

        [[nodiscard]] bool intersectsWith(const MemoryRegion& other) const {
            return this->addressRange.intersectsWith(other.addressRange);
        }

    private:
        static inline std::atomic<std::size_t> lastId = 0;
    };
}
