#pragma once

#include <cstdint>

#include "MemoryRegion.hpp"

namespace Bloom
{
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
    };
}
