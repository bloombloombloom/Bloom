#include "ReadMemory.hpp"

#include "src/DebugServers/GdbRsp/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServers/GdbRsp/ResponsePackets/ResponsePacket.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServers::Gdb::AvrGdb::CommandPackets
{
    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::ResponsePacket;

    using Exceptions::Exception;

    void ReadMemory::init() {
        if (this->data.size() < 4) {
            throw Exception("Invalid packet length");
        }

        auto packetString = QString::fromLocal8Bit(
            reinterpret_cast<const char*>(this->data.data() + 1),
            static_cast<int>(this->data.size() - 1)
        );

        /*
         * The read memory ('m') packet consists of two segments, an address and a number of bytes to read.
         * These are separated by a comma character.
         */
        auto packetSegments = packetString.split(",");

        if (packetSegments.size() != 2) {
            throw Exception(
                "Unexpected number of segments in packet data: " + std::to_string(packetSegments.size())
            );
        }

        bool conversionStatus = false;
        this->startAddress = packetSegments.at(0).toUInt(&conversionStatus, 16);

        if (!conversionStatus) {
            throw Exception("Failed to parse start address from read memory packet data");
        }

        this->bytes = packetSegments.at(1).toUInt(&conversionStatus, 16);

        if (!conversionStatus) {
            throw Exception("Failed to parse read length from read memory packet data");
        }
    }

    void ReadMemory::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling ReadMemory packet");

        try {
            auto memoryType = this->getMemoryTypeFromGdbAddress(this->startAddress);
            auto startAddress = this->removeMemoryTypeIndicatorFromGdbAddress(this->startAddress);
            auto memoryBuffer = targetControllerConsole.readMemory(memoryType, startAddress, this->bytes);

            auto hexMemoryBuffer = Packet::dataToHex(memoryBuffer);
            debugSession.connection.writePacket(
                ResponsePacket(std::vector<unsigned char>(hexMemoryBuffer.begin(), hexMemoryBuffer.end()))
            );

        } catch (const Exception& exception) {
            Logger::error("Failed to read memory from target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
