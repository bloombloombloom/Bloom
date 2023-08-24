#pragma once

#include <optional>

#include "src/Targets/TargetMemory.hpp"
#include "HexViewerWidgetSettings.hpp"

namespace Widgets
{
    class ByteItem;

    struct HexViewerSharedState
    {
    public:
        const Targets::TargetMemoryDescriptor& memoryDescriptor;
        const std::optional<Targets::TargetMemoryBuffer>& data;

        HexViewerWidgetSettings& settings;

        ByteItem* hoveredByteItem = nullptr;
        std::optional<Targets::TargetStackPointer> currentStackPointer;
        bool highlightingEnabled = false;
        std::set<Targets::TargetMemoryAddressRange> highlightedAddressRanges;

        HexViewerSharedState(
            const Targets::TargetMemoryDescriptor& memoryDescriptor,
            const std::optional<Targets::TargetMemoryBuffer>& data,
            HexViewerWidgetSettings& settings
        )
            : memoryDescriptor(memoryDescriptor)
            , data(data)
            , settings(settings)
        {}
    };
}
