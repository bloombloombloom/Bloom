#include "WchLinkProgrammingInterface.hpp"

#include "FlashProgramOpcodes.hpp"

#include "src/Services/StringService.hpp"
#include "src/Targets/TargetDescription/Exceptions/InvalidTargetDescriptionDataException.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink
{
    using namespace ::DebugToolDrivers::Protocols::RiscVDebugSpec;
    using namespace Exceptions;

    using DebugModule::DmiOperation;

    WchLinkProgrammingInterface::WchLinkProgrammingInterface(
        WchLinkInterface& wchLinkInterface,
        const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile
    )
        : wchLinkInterface(wchLinkInterface)
        , targetDescriptionFile(targetDescriptionFile)
        , flashProgramOpcodes(
            WchLinkProgrammingInterface::getFlashProgramOpcodes(
                this->targetDescriptionFile.getProperty("wch_link_interface", "programming_opcode_key").value
            )
        )
        , programmingPacketSize(
            Services::StringService::toUint32(
                this->targetDescriptionFile.getProperty("wch_link_interface", "programming_packet_size").value
            )
        )
    {}

    std::optional<Targets::TargetMemorySize> WchLinkProgrammingInterface::alignmentSize(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemorySize bufferSize
    ) {
        return bufferSize > WchLinkProgrammingInterface::MAX_PARTIAL_PAGE_WRITE_SIZE
            ? std::optional{this->programmingPacketSize}
            : std::nullopt;
    }

    void WchLinkProgrammingInterface::writeProgramMemory(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor,
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemoryBufferSpan buffer
    ) {
        if (buffer.size() <= WchLinkProgrammingInterface::MAX_PARTIAL_PAGE_WRITE_SIZE) {
            return this->wchLinkInterface.writePartialPage(startAddress, buffer);
        }

        this->wchLinkInterface.writeFullPage(
            startAddress,
            buffer,
            this->programmingPacketSize,
            this->flashProgramOpcodes
        );
    }

    void WchLinkProgrammingInterface::eraseProgramMemory(
        const Targets::TargetAddressSpaceDescriptor& addressSpaceDescriptor,
        const Targets::TargetMemorySegmentDescriptor& memorySegmentDescriptor
    ) {
        this->wchLinkInterface.eraseChip();
    }

    std::span<const unsigned char> WchLinkProgrammingInterface::getFlashProgramOpcodes(const std::string& key) {
        if (key == "op1") {
            return FlashProgramOpcodes::FLASH_OP1;
        }

        if (key == "op2") {
            return FlashProgramOpcodes::FLASH_OP2;
        }

        throw Targets::TargetDescription::Exceptions::InvalidTargetDescriptionDataException{
            "Invalid programming_opcode_key value (\"" + key + "\")"
        };
    }
}
