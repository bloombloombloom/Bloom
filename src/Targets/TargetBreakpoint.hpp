#pragma once

#include <cstdint>
#include <optional>

#include "TargetMemory.hpp"

namespace Targets
{
    enum class TargetBreakCause: std::uint8_t
    {
        BREAKPOINT,
        UNKNOWN,
    };

    struct TargetBreakpoint
    {
        enum class Type: std::uint8_t
        {
            HARDWARE,
            SOFTWARE,
        };

        /**
         * Byte address of the breakpoint.
         */
        TargetMemoryAddress address = 0;

        Type type = Type::SOFTWARE;

        TargetBreakpoint() = default;

        explicit TargetBreakpoint(TargetMemoryAddress address, Type type = Type::SOFTWARE)
            : address(address)
            , type(type)
        {};
    };

    struct BreakpointResources
    {
        std::optional<std::uint16_t> maximumHardwareBreakpoints;
        std::optional<std::uint16_t> maximumSoftwareBreakpoints;

        BreakpointResources(
            std::optional<std::uint16_t> maximumHardwareBreakpoints,
            std::optional<std::uint16_t> maximumSoftwareBreakpoints
        )
            : maximumHardwareBreakpoints(maximumHardwareBreakpoints)
            , maximumSoftwareBreakpoints(maximumSoftwareBreakpoints)
        {}
    };
}
