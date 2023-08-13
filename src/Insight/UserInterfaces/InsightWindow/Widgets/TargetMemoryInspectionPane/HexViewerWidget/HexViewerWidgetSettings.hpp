#pragma once

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/TargetMemoryInspectionPane/AddressType.hpp"

namespace Widgets
{
    struct HexViewerWidgetSettings
    {
        bool groupStackMemory = true;
        bool highlightFocusedMemory = true;
        bool highlightHoveredRowAndCol = true;
        bool displayAsciiValues = false;
        bool displayAnnotations = true;

        AddressType addressLabelType = AddressType::ABSOLUTE;
    };
}
