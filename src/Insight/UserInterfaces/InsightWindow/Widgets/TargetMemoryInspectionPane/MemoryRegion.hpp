#pragma once

#include <cstdint>
#include <atomic>
#include <QString>
#include <QJsonObject>
#include <utility>

#include "src/Targets/TargetMemory.hpp"
#include "AddressType.hpp"

#include "src/Services/DateTimeService.hpp"

#include "src/Helpers/BiMap.hpp"

enum class MemoryRegionType: std::uint8_t
{
    FOCUSED,
    EXCLUDED,
};

enum class MemoryRegionAddressInputType: std::uint8_t
{
    ABSOLUTE,
    RELATIVE,
};

class MemoryRegion
{
public:
    QString name;
    QDateTime createdDate = Services::DateTimeService::currentDateTime();
    Targets::TargetMemoryType memoryType;
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
    AddressType addressRangeInputType = AddressType::ABSOLUTE;

    /**
     * This address range will always be in absolute form. Regardless of the value of this->addressRangeType.
     * See the comment above, for this->addressRangeType, for more.
     */
    Targets::TargetMemoryAddressRange addressRange;

    MemoryRegion(
        const QString& name,
        Targets::TargetMemoryType memoryType,
        MemoryRegionType type,
        const Targets::TargetMemoryAddressRange& addressRange
    );

    MemoryRegion(const QJsonObject& jsonObject);

    virtual QJsonObject toJson() const;

    virtual ~MemoryRegion() = default;

    MemoryRegion(const MemoryRegion& other) = default;
    MemoryRegion(MemoryRegion&& other) = default;

    MemoryRegion& operator = (const MemoryRegion& other) = default;
    MemoryRegion& operator = (MemoryRegion&& other) = default;

    [[nodiscard]] bool intersectsWith(const MemoryRegion& other) const {
        return this->addressRange.intersectsWith(other.addressRange);
    }

private:
    static const inline BiMap<MemoryRegionType, QString> memoryRegionTypesByName = {
        {MemoryRegionType::EXCLUDED, "excluded"},
        {MemoryRegionType::FOCUSED, "focused"},
    };

    static const inline BiMap<AddressType, QString> addressTypesByName = {
        {AddressType::ABSOLUTE, "absolute"},
        {AddressType::RELATIVE, "relative"},
    };
};
