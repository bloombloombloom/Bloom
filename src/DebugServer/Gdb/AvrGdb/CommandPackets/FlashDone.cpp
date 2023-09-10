#include "FlashDone.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::OkResponsePacket;

    using namespace Exceptions;

    FlashDone::FlashDone(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {}

    void FlashDone::handle(Gdb::DebugSession& debugSession, TargetControllerService& targetControllerService) {
        Logger::info("Handling FlashDone packet");

        try {
            if (debugSession.programmingSession.has_value()) {
                const auto& programmingSession = debugSession.programmingSession.value();
                Logger::info(
                    "Flushing " + std::to_string(programmingSession.buffer.size()) + " bytes to target's program memory"
                );

                targetControllerService.enableProgrammingMode();

                targetControllerService.writeMemory(
                    Targets::TargetMemoryType::FLASH,
                    programmingSession.startAddress,
                    std::move(programmingSession.buffer)
                );

                debugSession.programmingSession.reset();
            }

            Logger::warning("Program memory updated");
            targetControllerService.disableProgrammingMode();

            Logger::warning("Resetting target");
            targetControllerService.resetTarget();
            Logger::info("Target reset complete");

            debugSession.connection.writePacket(OkResponsePacket());

        } catch (const Exception& exception) {
            Logger::error("Failed to handle FlashDone packet - " + exception.getMessage());
            debugSession.programmingSession.reset();

            try {
                targetControllerService.disableProgrammingMode();

            } catch (const Exception& exception) {
                Logger::error("Failed to disable programming mode - " + exception.getMessage());
            }

            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
