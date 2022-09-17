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
        : CommandPacket(rawPacket)
    {
        const auto packetString = QString::fromLocal8Bit(
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
            const auto flashPageSize = debugSession.gdbTargetDescriptor.targetDescriptor.memoryDescriptorsByType.at(
                Targets::TargetMemoryType::FLASH
            ).pageSize.value();

            if ((this->bytes % flashPageSize) != 0) {
                throw Exception(
                    "Invalid erase size (" + std::to_string(this->bytes) + " bytes) provided by GDB - must be a "
                    "multiple of the target's flash page size (" + std::to_string(flashPageSize) + " bytes)"
                );
            }

            targetControllerConsole.enableProgrammingMode();

            targetControllerConsole.writeMemory(
                Targets::TargetMemoryType::FLASH,
                this->startAddress,
                Targets::TargetMemoryBuffer(this->bytes, 0xFF)
            );

            debugSession.connection.writePacket(OkResponsePacket());

        } catch (const Exception& exception) {
            Logger::error("Failed to erase flash memory - " + exception.getMessage());
            debugSession.programmingSession.reset();

            try {
                targetControllerConsole.disableProgrammingMode();

            } catch (const Exception& exception) {
                Logger::error("Failed to disable programming mode - " + exception.getMessage());
            }

            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
