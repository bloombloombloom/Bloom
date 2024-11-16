#pragma once

#include <cstdint>
#include <span>
#include <string>

#include "WchLinkInterface.hpp"
#include "src/Targets/RiscV/Wch/TargetDescriptionFile.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink
{
    /**
     * WCH debug tools cannot write to program memory via the target's RISC-V debug module, so we cannot program the
     * target via the tool's RISC-V DTM interface. Instead, the WCH-Link protocol provides a dedicated command for
     * writing to program memory, which is why this class implements the RISC-V programming interface.
     * See WchLinkInterface::writeFlashMemory() for more.
     */
    class WchLinkProgrammingInterface
        : public TargetInterfaces::RiscV::RiscVProgramInterface
    {
    public:
        WchLinkProgrammingInterface(
            WchLinkInterface& wchLinkInterface,
            const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile
        );

        std::optional<Targets::TargetMemorySize> alignmentSize(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemorySize bufferSize
        ) override;

        void writeProgramMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBufferSpan buffer
        ) override;

        void eraseProgramMemory(
            const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
            const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
        ) override;

    private:
        static constexpr Targets::TargetMemorySize MAX_PARTIAL_PAGE_WRITE_SIZE = 64;

        WchLinkInterface& wchLinkInterface;
        const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile;

        std::span<const unsigned char> flashProgramOpcodes;
        Targets::TargetMemorySize programmingPacketSize;

        static std::span<const unsigned char> getFlashProgramOpcodes(const std::string& key);
    };
}
