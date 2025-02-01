#pragma once

#include <cstdint>
#include <array>

#include "TargetMemory.hpp"
#include "TargetAddressSpaceDescriptor.hpp"
#include "TargetMemorySegmentDescriptor.hpp"

namespace Targets
{
    enum class TargetBreakCause: std::uint8_t
    {
        BREAKPOINT,
        UNKNOWN,
    };

    struct TargetProgramBreakpoint
    {
        static constexpr auto MAX_SIZE = TargetMemorySize{4};

        enum class Type: std::uint8_t
        {
            HARDWARE,
            SOFTWARE,
        };

        const TargetAddressSpaceDescriptor& addressSpaceDescriptor;
        const TargetMemorySegmentDescriptor& memorySegmentDescriptor;
        TargetMemoryAddress address;
        TargetMemorySize size;
        Type type;
        std::array<unsigned char, TargetProgramBreakpoint::MAX_SIZE> originalData;
    };

    struct BreakpointResources
    {
        std::uint32_t hardwareBreakpoints = 0;
        std::uint32_t softwareBreakpoints = 0;
        std::uint32_t reservedHardwareBreakpoints = 0;
    };
}
