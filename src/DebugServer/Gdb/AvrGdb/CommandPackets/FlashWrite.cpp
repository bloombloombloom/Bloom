#include "FlashWrite.hpp"

#include <QByteArray>

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

    FlashWrite::FlashWrite(const RawPacketType& rawPacket)
        : MemoryAccessCommandPacket(rawPacket)
    {
        if (this->data.size() < 15) {
            throw Exception("Invalid packet length");
        }

        /*
         * The flash write ('vFlashWrite') packet consists of two segments, an address and a buffer.
         *
         * Seperated by a colon.
         */
        auto colonIt = std::find(this->data.begin() + 12, this->data.end(), ':');

        if (colonIt == this->data.end()) {
            throw Exception("Failed to find colon delimiter in write flash packet.");
        }

        bool conversionStatus = false;
        this->startAddress = QByteArray(
            reinterpret_cast<const char*>(this->data.data() + 12),
            std::distance(this->data.begin(), colonIt) - 12
        ).toUInt(&conversionStatus, 16);

        if (!conversionStatus) {
            throw Exception("Failed to parse start address from flash write packet data");
        }

        this->buffer = Targets::TargetMemoryBuffer(colonIt + 1, this->data.end());
    }

    void FlashWrite::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling FlashWrite packet");

        try {
            targetControllerConsole.writeMemory(
                Targets::TargetMemoryType::FLASH,
                this->startAddress,
                this->buffer
            );

            debugSession.connection.writePacket(OkResponsePacket());

        } catch (const Exception& exception) {
            Logger::error("Failed to write to flash memory - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
