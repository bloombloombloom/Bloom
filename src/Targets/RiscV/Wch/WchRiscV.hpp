#pragma once

#include <cstdint>
#include <optional>
#include <functional>

#include "src/Targets/RiscV/RiscV.hpp"

#include "TargetDescriptionFile.hpp"

namespace Targets::RiscV::Wch
{
    class WchRiscV: public ::Targets::RiscV::RiscV
    {
    public:
        WchRiscV(const TargetConfig& targetConfig, TargetDescriptionFile&& targetDescriptionFile);

        void activate() override;
        void postActivate() override;
        TargetDescriptor targetDescriptor() override;

        void setProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) override;
        void removeProgramBreakpoint(const TargetProgramBreakpoint& breakpoint) override;

        void writeMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            TargetMemoryAddress startAddress,
            TargetMemoryBufferSpan buffer
        ) override;

    protected:
        TargetDescriptionFile targetDescriptionFile;
        std::optional<std::reference_wrapper<const TargetDescription::Variant>> variant = std::nullopt;

        const TargetMemorySegmentDescriptor& programMemorySegmentDescriptor;
        const TargetMemorySegmentDescriptor& bootProgramMemorySegmentDescriptor;
        const TargetMemorySegmentDescriptor& mappedProgramMemorySegmentDescriptor;

        const TargetMemorySegmentDescriptor& getDestinationProgramMemorySegmentDescriptor();
        TargetMemoryAddress transformAliasedProgramMemoryAddress(TargetMemoryAddress address) const;
    };
}
