#pragma once

#include <cstdint>

#include "TargetMemory.hpp"

namespace Bloom::Targets
{
    enum class TargetBreakCause: int
    {
        BREAKPOINT,
        UNKNOWN,
    };

    struct TargetBreakpoint
    {
        /**
         * Byte address of the breakpoint.
         */
        TargetMemoryAddress address = 0;

        TargetBreakpoint() = default;
        explicit TargetBreakpoint(TargetMemoryAddress address): address(address) {};
    };
}
