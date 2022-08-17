#pragma once

namespace Bloom::Widgets
{
    struct PanelState
    {
        int size = 0;
        bool open = false;

        PanelState() = default;
        PanelState(int size, bool open): size(size), open(open) {};
    };
}
