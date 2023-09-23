#pragma once

#include <cstdint>
#include <optional>
#include <cassert>

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
        std::uint16_t reservedHardwareBreakpoints;

        BreakpointResources(
            std::optional<std::uint16_t> maximumHardwareBreakpoints,
            std::optional<std::uint16_t> maximumSoftwareBreakpoints,
            std::uint16_t reservedHardwareBreakpoints
        )
            : maximumHardwareBreakpoints(maximumHardwareBreakpoints)
            , maximumSoftwareBreakpoints(maximumSoftwareBreakpoints)
            , reservedHardwareBreakpoints(reservedHardwareBreakpoints)
        {
            assert(
                !this->maximumHardwareBreakpoints.has_value()
                || this->maximumHardwareBreakpoints >= this->reservedHardwareBreakpoints
            );
        }
    };
}
