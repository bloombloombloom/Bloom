#pragma once

#include <cstdint>
#include <optional>

#include "src/Targets/TargetMemory.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

namespace DebugToolDrivers::TargetInterfaces::RiscV
{
    class RiscVProgramInterface
    {
    public:
        virtual std::optional<Targets::TargetMemorySize> alignmentSize(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bufferSize
        ) = 0;

        virtual void writeProgramMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBufferSpan buffer
        ) = 0;

        virtual void eraseProgramMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
        ) = 0;
    };
}
