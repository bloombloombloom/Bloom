#include "ResetTarget.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using TargetController::TargetControllerConsole;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::ResponsePacket;

    using Exceptions::Exception;

    ResetTarget::ResetTarget(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void ResetTarget::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling ResetTarget packet");

        try {
            targetControllerConsole.resetTarget();

            debugSession.connection.writePacket(ResponsePacket(Packet::toHex(
                "Target reset complete - use the 'continue' command to begin execution.\n"
            )));

        } catch (const Exception& exception) {
            Logger::error("Failed to reset target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
