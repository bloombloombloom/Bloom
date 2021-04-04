#pragma once

#include <cstdint>

namespace Bloom::Targets
{
    enum class TargetBreakCause: int
    {
        BREAKPOINT,
        UNKNOWN,
    };

    using TargetBreakpointAddress = std::uint32_t;

    struct TargetBreakpoint
    {
        /**
         * Byte address of the breakpoint.
         */
        TargetBreakpointAddress address;

        bool disabled = false;
    };
}

