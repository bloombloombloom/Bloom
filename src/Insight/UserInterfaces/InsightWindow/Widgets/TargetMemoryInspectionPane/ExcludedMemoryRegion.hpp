#pragma once

#include "MemoryRegion.hpp"

namespace Bloom
{
    class ExcludedMemoryRegion: public MemoryRegion
    {
    public:
        ExcludedMemoryRegion(
            const QString& name,
            Targets::TargetMemoryType memoryType,
            const Targets::TargetMemoryAddressRange& addressRange
        );

        ExcludedMemoryRegion(const QJsonObject& jsonObject);
    };
}
