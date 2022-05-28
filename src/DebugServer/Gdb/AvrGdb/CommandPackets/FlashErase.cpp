#include "FlashErase.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    using TargetController::TargetControllerConsole;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::OkResponsePacket;

    using namespace Bloom::Exceptions;

    FlashErase::FlashErase(const RawPacketType& rawPacket)
        : MemoryAccessCommandPacket(rawPacket)
    {
        auto packetString = QString::fromLocal8Bit(
            reinterpret_cast<const char*>(this->data.data() + 12),
            static_cast<int>(this->data.size() - 12)
        );

        /*
         * The flash erase ('vFlashErase') packet consists of two segments, an address and a length, separated by a
         * comma.
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
            throw Exception("Failed to parse start address from flash erase packet data");
        }

        this->bytes = packetSegments.at(1).toUInt(&conversionStatus, 16);

        if (!conversionStatus) {
            throw Exception("Failed to parse length from flash erase packet data");
        }
    }

    void FlashErase::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling FlashErase packet");

        try {
            targetControllerConsole.writeMemory(
                Targets::TargetMemoryType::FLASH,
                this->startAddress,
                Targets::TargetMemoryBuffer(this->bytes, 0xFF)
            );

            debugSession.connection.writePacket(OkResponsePacket());

        } catch (const Exception& exception) {
            Logger::error("Failed to erase flash memory - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
