#pragma once

#include <cstdint>
#include <set>
#include <vector>
#include <optional>

#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
#include "src/Targets/TargetState.hpp"
#include "src/Targets/TargetRegisterDescriptor.hpp"
#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

namespace DebugToolDrivers::TargetInterfaces::RiscV
{
    class RiscVDebugInterface
    {
    public:
        virtual void activate() = 0;
        virtual void deactivate() = 0;

        virtual std::string getDeviceId() = 0;

        virtual Targets::TargetExecutionState getExecutionState() = 0;

        virtual void stop() = 0;
        virtual void run() = 0;
        virtual void step() = 0;
        virtual void reset() = 0;

        virtual Targets::BreakpointResources getBreakpointResources() = 0;
        virtual void setProgramBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint) = 0;
        virtual void removeProgramBreakpoint(const Targets::TargetProgramBreakpoint& breakpoint) = 0;

        virtual Targets::TargetRegisterDescriptorAndValuePairs readCpuRegisters(
            const Targets::TargetRegisterDescriptors& descriptors
        ) = 0;
        virtual void writeCpuRegisters(const Targets::TargetRegisterDescriptorAndValuePairs& registers) = 0;

        virtual Targets::TargetMemoryBuffer readMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bytes,
            const std::set<Targets::TargetMemoryAddressRange>& excludedAddressRanges = {}
        ) = 0;
        virtual void writeMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBufferSpan buffer
        ) = 0;
        virtual void eraseMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
        ) = 0;

        virtual void enableProgrammingMode() = 0;
        virtual void disableProgrammingMode() = 0;
    };
}
