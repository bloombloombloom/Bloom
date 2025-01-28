#pragma once

#include <string>
#include <cstdint>
#include <optional>
#include <utility>

#include "TargetMemory.hpp"
#include "TargetMemoryAddressRange.hpp"
#include "TargetMemorySegmentType.hpp"

namespace Targets
{
    struct TargetMemorySegmentDescriptor
    {
    public:
        /*
         * The ID of a memory segment is just a hash of the address space key and the memory segment key, concatenated,
         * which will be unique.
         *
         * We use the ID for cheap equality checks.
         */
        const TargetMemorySegmentId id;
        const std::string key;
        std::string name;
        TargetMemorySegmentType type;
        TargetMemoryAddressRange addressRange;
        std::uint8_t addressSpaceUnitSize;
        bool executable;
        TargetMemoryAccess debugModeAccess;
        TargetMemoryAccess programmingModeAccess;
        bool inspectionEnabled;
        std::optional<TargetMemorySize> pageSize;

        TargetMemorySegmentDescriptor(
            const std::string& addressSpaceKey,
            const std::string& key,
            const std::string& name,
            TargetMemorySegmentType type,
            const TargetMemoryAddressRange& addressRange,
            std::uint8_t addressSpaceUnitSize,
            bool executable,
            const TargetMemoryAccess& debugModeAccess,
            const TargetMemoryAccess& programmingModeAccess,
            bool inspectionEnabled,
            std::optional<TargetMemorySize> pageSize
        );

        TargetMemorySegmentDescriptor& operator = (const TargetMemorySegmentDescriptor& other) = delete;

        TargetMemorySegmentDescriptor(TargetMemorySegmentDescriptor&& other) noexcept = default;

        bool operator == (const TargetMemorySegmentDescriptor& other) const;
        bool operator != (const TargetMemorySegmentDescriptor& other) const;

        TargetMemorySize size() const;

        [[nodiscard]] TargetMemorySegmentDescriptor clone() const;

    private:
        TargetMemorySegmentDescriptor(const TargetMemorySegmentDescriptor& other) = default;
    };
}
