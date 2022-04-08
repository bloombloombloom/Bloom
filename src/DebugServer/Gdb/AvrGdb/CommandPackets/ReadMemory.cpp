#include "ReadMemory.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::ResponsePacket;

    using Exceptions::Exception;

    ReadMemory::ReadMemory(const RawPacketType& rawPacket)
        : MemoryAccessCommandPacket(rawPacket)
    {
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
        const auto gdbStartAddress = packetSegments.at(0).toUInt(&conversionStatus, 16);

        if (!conversionStatus) {
            throw Exception("Failed to parse start address from read memory packet data");
        }

        this->memoryType = this->getMemoryTypeFromGdbAddress(gdbStartAddress);
        this->startAddress = this->removeMemoryTypeIndicatorFromGdbAddress(gdbStartAddress);

        this->bytes = packetSegments.at(1).toUInt(&conversionStatus, 16);

        if (!conversionStatus) {
            throw Exception("Failed to parse read length from read memory packet data");
        }
    }

    void ReadMemory::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling ReadMemory packet");

        try {
            auto memoryBuffer = targetControllerConsole.readMemory(
                this->memoryType,
                this->startAddress,
                this->bytes
            );

            debugSession.connection.writePacket(
                ResponsePacket(Packet::toHex(memoryBuffer))
            );

        } catch (const Exception& exception) {
            Logger::error("Failed to read memory from target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
