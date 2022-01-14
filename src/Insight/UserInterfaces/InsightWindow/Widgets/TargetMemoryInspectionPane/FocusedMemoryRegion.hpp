#pragma once

#include <cstdint>

#include "MemoryRegion.hpp"

namespace Bloom
{
    enum class MemoryRegionDataType: std::uint8_t
    {
        UNKNOWN,
        UNSIGNED_INTEGER,
        ASCII_STRING,
    };

    class FocusedMemoryRegion: public MemoryRegion
    {
    public:
        MemoryRegionDataType dataType = MemoryRegionDataType::UNKNOWN;

        explicit FocusedMemoryRegion(
            const QString& name,
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            const Targets::TargetMemoryAddressRange& addressRange
        ): MemoryRegion(name, MemoryRegionType::FOCUSED, memoryDescriptor, addressRange) {};
    };
}
