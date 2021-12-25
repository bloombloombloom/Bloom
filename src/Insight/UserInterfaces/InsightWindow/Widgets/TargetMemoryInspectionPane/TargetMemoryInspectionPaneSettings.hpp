#pragma once

#include <vector>

#include "FocusedMemoryRegion.hpp"
#include "ExcludedMemoryRegion.hpp"

#include "HexViewerWidget/HexViewerWidgetSettings.hpp"

namespace Bloom::Widgets
{
    struct TargetMemoryInspectionPaneSettings
    {
        std::vector<FocusedMemoryRegion> focusedMemoryRegions;
        std::vector<ExcludedMemoryRegion> excludedMemoryRegions;

        HexViewerWidgetSettings hexViewerWidgetSettings;
    };
}
