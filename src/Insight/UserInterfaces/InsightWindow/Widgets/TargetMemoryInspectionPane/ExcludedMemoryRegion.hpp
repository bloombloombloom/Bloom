#pragma once

#include "MemoryRegion.hpp"

class ExcludedMemoryRegion: public MemoryRegion
{
public:
    ExcludedMemoryRegion(
        const QString& name,
        const Targets::TargetMemoryAddressRange& addressRange
    );

    explicit ExcludedMemoryRegion(const QJsonObject& jsonObject);
};
