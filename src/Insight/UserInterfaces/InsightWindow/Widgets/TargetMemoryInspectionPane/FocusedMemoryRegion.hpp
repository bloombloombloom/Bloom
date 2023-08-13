#pragma once

#include <cstdint>

#include "MemoryRegion.hpp"

#include "src/Helpers/BiMap.hpp"

enum class MemoryRegionDataType: std::uint8_t
{
    UNKNOWN,
    UNSIGNED_INTEGER,
    SIGNED_INTEGER,
    ASCII_STRING,
};

class FocusedMemoryRegion: public MemoryRegion
{
public:
    MemoryRegionDataType dataType = MemoryRegionDataType::UNKNOWN;
    Targets::TargetMemoryEndianness endianness = Targets::TargetMemoryEndianness::LITTLE;

    FocusedMemoryRegion(
        const QString& name,
        Targets::TargetMemoryType memoryType,
        const Targets::TargetMemoryAddressRange& addressRange
    );

    FocusedMemoryRegion(const QJsonObject& jsonObject);

    QJsonObject toJson() const override;

private:
    static const inline BiMap<MemoryRegionDataType, QString> regionDataTypesByName = {
        {MemoryRegionDataType::UNKNOWN, "other"},
        {MemoryRegionDataType::UNSIGNED_INTEGER, "unsigned_int"},
        {MemoryRegionDataType::SIGNED_INTEGER, "signed_int"},
        {MemoryRegionDataType::ASCII_STRING, "ascii_string"},
    };

    static const inline BiMap<Targets::TargetMemoryEndianness, QString> regionEndiannessByName = {
        {Targets::TargetMemoryEndianness::LITTLE, "little"},
        {Targets::TargetMemoryEndianness::BIG, "big"},
    };
};
