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
        TargetMemoryAddress startAddress;
        TargetMemorySize size;
        TargetMemoryAccess debugModeAccess;
        TargetMemoryAccess programmingModeAccess;
        std::optional<TargetMemorySize> pageSize;

        TargetMemorySegmentDescriptor(
            const std::string& key,
            const std::string& name,
            TargetMemorySegmentType type,
            TargetMemoryAddress startAddress,
            TargetMemorySize size,
            const TargetMemoryAccess& debugModeAccess,
            const TargetMemoryAccess& programmingModeAccess,
            std::optional<TargetMemorySize> pageSize
        )
            : key(key)
            , name(name)
            , type(type)
            , startAddress(startAddress)
            , size(size)
            , debugModeAccess(debugModeAccess)
            , programmingModeAccess(programmingModeAccess)
            , pageSize(pageSize)
        {};
    };
}
