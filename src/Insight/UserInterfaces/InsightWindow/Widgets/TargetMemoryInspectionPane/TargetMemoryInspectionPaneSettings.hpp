#pragma once

#include <vector>

#include "FocusedMemoryRegion.hpp"
#include "ExcludedMemoryRegion.hpp"

#include "HexViewerWidget/HexViewerWidgetSettings.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PanelState.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PaneState.hpp"

namespace Bloom::Widgets
{
    struct TargetMemoryInspectionPaneSettings
    {
        bool refreshOnTargetStop = false;
        bool refreshOnActivation = false;

        HexViewerWidgetSettings hexViewerWidgetSettings;

        std::vector<FocusedMemoryRegion> focusedMemoryRegions;
        std::vector<ExcludedMemoryRegion> excludedMemoryRegions;

        PanelState rightPanelState = PanelState(300, true);
        PaneState snapshotManagerState = PaneState(true, true, std::nullopt);
    };
}
