#pragma once

#include <cstdint>
#include <optional>
#include <functional>

#include "src/Targets/RiscV/RiscV.hpp"

#include "WchRiscVTargetConfig.hpp"
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

        TargetMemoryBuffer readMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            TargetMemoryAddress startAddress,
            TargetMemorySize bytes,
            const std::set<TargetMemoryAddressRange>& excludedAddressRanges
        ) override;
        void writeMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            TargetMemoryAddress startAddress,
            TargetMemoryBufferSpan buffer
        ) override;
        void eraseMemory(
            const TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const TargetMemorySegmentDescriptor& memorySegmentDescriptor
        ) override;

        TargetMemoryAddress getProgramCounter() override;

    protected:
        WchRiscVTargetConfig targetConfig;
        TargetDescriptionFile targetDescriptionFile;
        std::optional<std::reference_wrapper<const TargetDescription::Variant>> variant = std::nullopt;

        const TargetMemorySegmentDescriptor& mappedSegmentDescriptor;
        const TargetMemorySegmentDescriptor& mainProgramSegmentDescriptor;
        const TargetMemorySegmentDescriptor& bootProgramSegmentDescriptor;
        const TargetMemorySegmentDescriptor& peripheralSegmentDescriptor;

        /*
         * The selected program segment is the program memory segment the user has selected to be the subject of all
         * memory accesses that are forwarded from the mapped program memory segment.
         *
         * In other words, whenever we service a memory access via the mapped program memory segment, we perform the
         * operation on the selected program segment, instead. This prevents memory access requests from being
         * misinterpreted in cases where the WCH target has switched to/from boot mode.
         *
         * The user can use the "program_segment_key" config param to specify the selected segment. If not provided,
         * the selected segment defaults to the main program memory segment. This means, by default, we assume the user
         * intends to debug the main user program, residing on the main program segment, as opposed to the boot program
         * residing on the boot segment. If the user intends to debug their boot program, they must select the boot
         * segment as the program segment, via the "program_segment_key" config param.
         *
         * I will explain this more clearly in the user documentation, on the Bloom website.
         *
         * For more, see the implementation of the memory access member functions.
         */
        const TargetMemorySegmentDescriptor& selectedProgramSegmentDescriptor;

        const TargetMemorySegmentDescriptor& resolveAliasedMemorySegment();
        TargetMemoryAddress transformMappedAddress(
            TargetMemoryAddress address,
            const TargetMemorySegmentDescriptor& segmentDescriptor
        );
    };
}
