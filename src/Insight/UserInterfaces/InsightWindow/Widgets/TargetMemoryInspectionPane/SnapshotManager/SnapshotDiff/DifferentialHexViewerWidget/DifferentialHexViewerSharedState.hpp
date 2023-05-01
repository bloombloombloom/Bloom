#pragma once

namespace Bloom::Widgets
{
    struct DifferentialHexViewerSharedState
    {
        bool syncingSettings = false;
        bool syncingScroll = false;
        bool syncingHover = false;
        bool syncingSelection = false;
    };
}
