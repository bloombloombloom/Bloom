#pragma once

#include <unordered_set>

namespace Widgets
{
    struct DifferentialHexViewerSharedState
    {
        std::unordered_set<Targets::TargetMemoryAddress> differences;

        bool syncingSettings = false;
        bool syncingScroll = false;
        bool syncingHover = false;
        bool syncingSelection = false;
    };
}
