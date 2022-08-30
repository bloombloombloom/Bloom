#pragma once

#include <vector>

#include "FocusedMemoryRegion.hpp"
#include "ExcludedMemoryRegion.hpp"

#include "HexViewerWidget/HexViewerWidgetSettings.hpp"

namespace Bloom::Widgets
{
    struct TargetMemoryInspectionPaneSettings
    {
        bool refreshOnTargetStop = false;
        bool refreshOnActivation = false;

        HexViewerWidgetSettings hexViewerWidgetSettings;

        std::vector<FocusedMemoryRegion> focusedMemoryRegions;
        std::vector<ExcludedMemoryRegion> excludedMemoryRegions;
    };
}
