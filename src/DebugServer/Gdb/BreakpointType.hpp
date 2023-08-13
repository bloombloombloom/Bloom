#pragma once

namespace DebugServer::Gdb
{
    enum class BreakpointType: int
    {
        UNKNOWN = 0,
        SOFTWARE_BREAKPOINT = 1,
        HARDWARE_BREAKPOINT = 2,
    };
}
