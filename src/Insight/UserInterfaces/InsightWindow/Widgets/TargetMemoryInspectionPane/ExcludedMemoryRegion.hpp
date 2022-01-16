#pragma once

#include "MemoryRegion.hpp"

namespace Bloom
{
    class ExcludedMemoryRegion: public MemoryRegion
    {
    public:
        explicit ExcludedMemoryRegion(
            const QString& name,
            const Targets::TargetMemoryAddressRange& addressRange
        ): MemoryRegion(name, MemoryRegionType::EXCLUDED, addressRange) {};
    };
}
