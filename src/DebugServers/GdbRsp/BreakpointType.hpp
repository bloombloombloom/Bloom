#pragma once

namespace Bloom::DebugServers::Gdb
{
    enum class BreakpointType: int
    {
        UNKNOWN = 0,
        SOFTWARE_BREAKPOINT = 1,
        HARDWARE_BREAKPOINT = 2,
    };
}
