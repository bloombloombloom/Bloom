#pragma once

#include <cstdint>
#include <optional>

namespace Bloom::Widgets
{
    struct HexViewerWidgetSettings
    {
        bool highlightStackMemory = false;
        std::optional<std::uint32_t> stackPointerAddress;

        bool highlightHoveredRowAndCol = false;
        bool highlightFocusedMemory = false;
        bool displayAsciiValues = false;
    };
}
