#include "InterruptExecution.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/TargetStopped.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/Signal.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::TargetStopped;
    using ResponsePackets::ErrorResponsePacket;
    using Exceptions::Exception;

    void InterruptExecution::handle(DebugSession& debugSession, TargetControllerService& targetControllerService) {
        Logger::debug("Handling InterruptExecution packet");

        if (targetControllerService.getTargetState() == Targets::TargetState::STOPPED) {
            debugSession.pendingInterrupt = true;
            return;
        }

        try {
            targetControllerService.stopTargetExecution();
            debugSession.connection.writePacket(TargetStopped(Signal::INTERRUPTED));

        } catch (const Exception& exception) {
            Logger::error("Failed to interrupt execution - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }

        /*
         * Whenever we respond to an interrupt signal, GDB always assumes that target execution has stopped. Even if we
         * respond with an error packet.
         *
         * Because of this, we always set the DebugSession::waitingForBreak flag to false, even if we failed to
         * interrupt target execution. This way, we won't end up sending an unexpected stop reply packet to GDB, when
         * the target does eventually stop (for some other reason).
         */
        debugSession.waitingForBreak = false;
    }
}
