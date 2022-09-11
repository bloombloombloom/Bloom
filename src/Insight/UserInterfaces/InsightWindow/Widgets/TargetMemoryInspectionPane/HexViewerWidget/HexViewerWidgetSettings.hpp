#pragma once

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/AddressType.hpp"

namespace Bloom::Widgets
{
    struct HexViewerWidgetSettings
    {
        bool highlightStackMemory = true;
        bool highlightFocusedMemory = true;
        bool highlightHoveredRowAndCol = true;
        bool displayAsciiValues = false;
        bool displayAnnotations = true;

        AddressType addressLabelType = AddressType::ABSOLUTE;
    };
}
