#include "ReadMemoryMap.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    using TargetController::TargetControllerConsole;

    using ResponsePackets::ResponsePacket;

    using Exceptions::Exception;

    ReadMemoryMap::ReadMemoryMap(const RawPacketType& rawPacket)
        : CommandPacket(rawPacket)
    {
        if (this->data.size() < 26) {
            throw Exception("Invalid packet length");
        }

        auto packetString = QString::fromLocal8Bit(
            reinterpret_cast<const char*>(this->data.data() + 23), // +23 to exclude the "qXfer:memory-map:read::"
            static_cast<int>(this->data.size() - 23)
        );

        /*
         * The read memory map ('qXfer:memory-map:read::...') packet consists of two segments, an offset and a length.
         * These are separated by a comma character.
         */
        auto packetSegments = packetString.split(",");

        if (packetSegments.size() != 2) {
            throw Exception(
                "Unexpected number of segments in packet data: " + std::to_string(packetSegments.size())
            );
        }

        bool conversionStatus = false;
        this->offset = packetSegments.at(0).toUInt(&conversionStatus, 16);

        if (!conversionStatus) {
            throw Exception("Failed to parse offset from read memory map packet data");
        }

        this->length = packetSegments.at(1).toUInt(&conversionStatus, 16);

        if (!conversionStatus) {
            throw Exception("Failed to parse read length from read memory map packet data");
        }
    }

    void ReadMemoryMap::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling ReadMemoryMap packet");

        using Targets::TargetMemoryType;
        const auto& memoryDescriptorsByType = debugSession.gdbTargetDescriptor.targetDescriptor.memoryDescriptorsByType;

        const auto& ramDescriptor = memoryDescriptorsByType.at(TargetMemoryType::RAM);
        const auto& flashDescriptor = memoryDescriptorsByType.at(TargetMemoryType::FLASH);

        const auto ramGdbOffset = debugSession.gdbTargetDescriptor.getMemoryOffset(
            Targets::TargetMemoryType::RAM
        );

        const auto flashGdbOffset = debugSession.gdbTargetDescriptor.getMemoryOffset(
            Targets::TargetMemoryType::FLASH
        );

        // We include the start address in ramSize to account for AVR registers in the data address space.
        const auto ramSize = ramDescriptor.size() + ramDescriptor.addressRange.startAddress;
        const auto flashSize = flashDescriptor.size();
        const auto flashPageSize = flashDescriptor.pageSize.value();

        const auto memoryMap =
            std::string("<memory-map>")
                + "<memory type=\"ram\" start=\"" + std::to_string(ramGdbOffset) + "\" length=\"" + std::to_string(ramSize) + "\"/>"
                + "<memory type=\"flash\" start=\"" + std::to_string(flashGdbOffset) + "\" length=\"" + std::to_string(flashSize) + "\">"
                    + "<property name=\"blocksize\">" + std::to_string(flashPageSize) + "</property>"
                + "</memory>"
            + "</memory-map>";

        if (this->offset < memoryMap.size() && this->length > 0) {
            auto memoryMapData = std::vector<unsigned char>(
                memoryMap.begin() + this->offset,
                memoryMap.begin() + std::min(
                    static_cast<long>(this->offset + this->length),
                    static_cast<long>(memoryMap.size())
                )
            );

            auto responseData = std::vector<unsigned char>({'l'});
            std::move(memoryMapData.begin(), memoryMapData.end(), std::back_inserter(responseData));

            debugSession.connection.writePacket(ResponsePacket(responseData));
            return;
        }

        debugSession.connection.writePacket(ResponsePacket(std::vector<unsigned char>({'l'})));
    }
}
