#pragma once

namespace Bloom::Widgets
{
    struct PaneState
    {
        bool activated = false;

        explicit PaneState(bool activated): activated(activated) {};
    };
}
