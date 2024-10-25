#include "ReadMemoryMap.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ResponsePacket;

    using Exceptions::Exception;

    ReadMemoryMap::ReadMemoryMap(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {
        using Services::StringService;

        if (this->data.size() < 26) {
            throw Exception{"Invalid packet length"};
        }

        /*
         * The read memory map ('qXfer:memory-map:read::...') packet consists of two segments, an offset and a length.
         * These are separated by a comma character.
         */
        const auto command = std::string{this->data.begin() + 23, this->data.end()};

        const auto delimiterPos = command.find_first_of(',');
        if (delimiterPos == std::string::npos) {
            throw Exception{"Invalid packet"};
        }

        this->offset = StringService::toUint32(command.substr(0, delimiterPos), 16);
        this->length = StringService::toUint32(command.substr(delimiterPos + 1), 16);
    }

    void ReadMemoryMap::handle(
        DebugSession& debugSession,
        const AvrGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling ReadMemoryMap packet");

        /*
         * We include register and EEPROM memory in our RAM section. This allows GDB to access registers and EEPROM
         * data via memory read/write packets.
         */
        const auto ramSectionEndAddress = gdbTargetDescriptor.translateTargetMemoryAddress(
            gdbTargetDescriptor.eepromMemorySegmentDescriptor.addressRange.endAddress,
            gdbTargetDescriptor.eepromAddressSpaceDescriptor,
            gdbTargetDescriptor.eepromMemorySegmentDescriptor
        );
        const auto ramSectionStartAddress = AvrGdbTargetDescriptor::SRAM_ADDRESS_MASK;
        const auto ramSectionSize = ramSectionEndAddress - ramSectionStartAddress + 1;

        const auto memoryMap =
                std::string{"<memory-map>"}
                + "<memory type=\"ram\" start=\"" + std::to_string(ramSectionStartAddress) + "\" length=\"" + std::to_string(ramSectionSize) + "\"/>"
                + "<memory type=\"flash\" start=\"0\" length=\"" + std::to_string(gdbTargetDescriptor.programMemorySegmentDescriptor.size()) + "\">"
                    + "<property name=\"blocksize\">" + std::to_string(gdbTargetDescriptor.programMemorySegmentDescriptor.pageSize.value()) + "</property>"
                + "</memory>"
            + "</memory-map>";

        auto responseData = std::vector<unsigned char>{'l'};

        if (this->offset < memoryMap.size() && this->length > 0) {
            responseData.insert(
                responseData.end(),
                memoryMap.begin() + this->offset,
                memoryMap.begin() + this->offset + std::min(
                    static_cast<long>(this->length),
                    static_cast<long>(memoryMap.size() - this->offset)
                )
            );
        }

        debugSession.connection.writePacket(ResponsePacket{responseData});
    }
}
