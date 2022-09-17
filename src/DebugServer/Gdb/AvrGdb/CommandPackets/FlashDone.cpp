#include "FlashDone.hpp"

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

    FlashDone::FlashDone(const RawPacketType& rawPacket)
        : CommandPacket(rawPacket)
    {}

    void FlashDone::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling FlashDone packet");

        try {
            if (debugSession.programmingSession.has_value()) {
                const auto& programmingSession = debugSession.programmingSession.value();

                targetControllerConsole.enableProgrammingMode();

                targetControllerConsole.writeMemory(
                    Targets::TargetMemoryType::FLASH,
                    programmingSession.startAddress,
                    std::move(programmingSession.buffer)
                );

                debugSession.programmingSession.reset();
            }

            targetControllerConsole.disableProgrammingMode();
            Logger::debug("Resetting target");
            targetControllerConsole.resetTarget();
            Logger::info("Target reset");

            debugSession.connection.writePacket(OkResponsePacket());

        } catch (const Exception& exception) {
            Logger::error("Failed to handle FlashDone packet - " + exception.getMessage());
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
