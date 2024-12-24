#pragma once

#include <QString>
#include <vector>

#include "FocusedMemoryRegion.hpp"
#include "ExcludedMemoryRegion.hpp"

#include "HexViewerWidget/HexViewerWidgetSettings.hpp"

#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PanelState.hpp"
#include "src/Insight/UserInterfaces/InsightWindow/Widgets/PaneState.hpp"

namespace Widgets
{
    struct TargetMemoryInspectionPaneSettings
    {
        QString addressSpaceKey;
        QString memorySegmentKey;
        bool refreshOnTargetStop = false;
        bool refreshOnActivation = false;

        HexViewerWidgetSettings hexViewerWidgetSettings;

        std::vector<FocusedMemoryRegion> focusedMemoryRegions;
        std::vector<ExcludedMemoryRegion> excludedMemoryRegions;

        PanelState rightPanelState = {300, true};
        PaneState snapshotManagerState = {true, true, std::nullopt};

        TargetMemoryInspectionPaneSettings(const QString& addressSpaceKey, const QString& memorySegmentKey)
            : addressSpaceKey(addressSpaceKey)
            , memorySegmentKey(memorySegmentKey)
        {}
    };
}
