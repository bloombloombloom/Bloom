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
        const Targets::TargetMemoryDescriptor& memoryDescriptor;
        Targets::TargetMemoryAddressRange addressRange;
        MemoryRegionAddressType addressRangeType = MemoryRegionAddressType::ABSOLUTE;

        MemoryRegion(
            QString name,
            MemoryRegionType type,
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            const Targets::TargetMemoryAddressRange& addressRange
        ): name(std::move(name)), type(type), memoryDescriptor(memoryDescriptor), addressRange(addressRange) {};

        bool operator == (const MemoryRegion& other) const {
            return this->id == other.id;
        }

        bool operator != (const MemoryRegion& other) const {
            return !(*this == other);
        }

        [[nodiscard]] bool intersectsWith(const MemoryRegion& other) const {
            return this->getAbsoluteAddressRange().intersectsWith(other.getAbsoluteAddressRange());
        }

        [[nodiscard]] Targets::TargetMemoryAddressRange getAbsoluteAddressRange() const;
        [[nodiscard]] Targets::TargetMemoryAddressRange getRelativeAddressRange() const;

    private:
        static inline std::atomic<std::size_t> lastId = 0;
    };
}
