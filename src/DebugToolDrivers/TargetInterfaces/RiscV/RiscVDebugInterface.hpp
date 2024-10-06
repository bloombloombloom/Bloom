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

namespace DebugToolDrivers::TargetInterfaces::RiscV
{
    class RiscVDebugInterface
    {
    public:
        virtual void init() = 0;
        virtual void activate() = 0;
        virtual void deactivate() = 0;

        virtual Targets::TargetExecutionState getExecutionState() = 0;

        virtual void stop() = 0;
        virtual void run() = 0;
        virtual void step() = 0;
        virtual void reset() = 0;

        virtual void setSoftwareBreakpoint(Targets::TargetMemoryAddress address) = 0;
        virtual void clearSoftwareBreakpoint(Targets::TargetMemoryAddress address) = 0;

        virtual std::uint16_t getHardwareBreakpointCount() = 0;
        virtual void setHardwareBreakpoint(Targets::TargetMemoryAddress address) = 0;
        virtual void clearHardwareBreakpoint(Targets::TargetMemoryAddress address) = 0;
        virtual void clearAllBreakpoints() = 0;

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
            const Targets::TargetMemoryBuffer& buffer
        ) = 0;
    };
}
