#pragma once

#include <vector>

#include "FocusedMemoryRegion.hpp"
#include "ExcludedMemoryRegion.hpp"

#include "HexViewerWidget/HexViewerWidgetSettings.hpp"

namespace Bloom::Widgets
{
    struct TargetMemoryInspectionPaneSettings
    {
        bool activated = false;
        HexViewerWidgetSettings hexViewerWidgetSettings;

        std::vector<FocusedMemoryRegion> focusedMemoryRegions;
        std::vector<ExcludedMemoryRegion> excludedMemoryRegions;
    };
}
