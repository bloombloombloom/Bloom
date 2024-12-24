#pragma once

#include <optional>

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

#include "HexViewerWidgetSettings.hpp"

namespace Widgets
{
    class ByteItem;

    struct HexViewerSharedState
    {
    public:
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor;

        const std::optional<Targets::TargetMemoryBuffer>& data;

        HexViewerWidgetSettings& settings;

        ByteItem* hoveredByteItem = nullptr;
        std::optional<Targets::TargetStackPointer> currentStackPointer;
        bool highlightingEnabled = false;
        std::set<Targets::TargetMemoryAddressRange> highlightedPrimaryAddressRanges;

        HexViewerSharedState(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            const std::optional<Targets::TargetMemoryBuffer>& data,
            HexViewerWidgetSettings& settings
        )
            : addressSpaceDescriptor(addressSpaceDescriptor)
            , memorySegmentDescriptor(memorySegmentDescriptor)
            , data(data)
            , settings(settings)
        {}
    };
}
