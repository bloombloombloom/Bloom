#include "InterruptExecution.hpp"

#include "src/DebugServers/GdbRsp/ResponsePackets/TargetStopped.hpp"
#include "src/DebugServers/GdbRsp/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServers/GdbRsp/Signal.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    using ResponsePackets::TargetStopped;
    using ResponsePackets::ErrorResponsePacket;
    using Exceptions::Exception;

    void InterruptExecution::handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) {
        Logger::debug("Handling InterruptExecution packet");

        try {
            targetControllerConsole.stopTargetExecution();
            debugSession.connection.writePacket(TargetStopped(Signal::INTERRUPTED));
            debugSession.waitingForBreak = false;

        } catch (const Exception& exception) {
            Logger::error("Failed to interrupt execution - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
