#pragma once

#include <string>
#include <optional>

#include "TargetMemory.hpp"
#include "TargetMemorySegmentType.hpp"

namespace Targets
{
    struct TargetMemorySegmentDescriptor
    {
    public:
        std::string key;
        std::string name;
        TargetMemorySegmentType type;
        TargetMemoryAddressRange addressRange;
        TargetMemoryAccess debugModeAccess;
        TargetMemoryAccess programmingModeAccess;
        std::optional<TargetMemorySize> pageSize;

        TargetMemorySegmentDescriptor(
            const std::string& key,
            const std::string& name,
            TargetMemorySegmentType type,
            const TargetMemoryAddressRange& addressRange,
            const TargetMemoryAccess& debugModeAccess,
            const TargetMemoryAccess& programmingModeAccess,
            std::optional<TargetMemorySize> pageSize
        )
            : key(key)
            , name(name)
            , type(type)
            , addressRange(addressRange)
            , debugModeAccess(debugModeAccess)
            , programmingModeAccess(programmingModeAccess)
            , pageSize(pageSize)
        {};

        TargetMemorySize size() const {
            return this->addressRange.size();
        }
    };
}
