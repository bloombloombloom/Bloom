#include "InterruptExecution.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/TargetStopped.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/Signal.hpp"

#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::TargetStopped;
    using ResponsePackets::ErrorResponsePacket;

    using ::Exceptions::Exception;

    void InterruptExecution::handle(DebugSession& debugSession, TargetControllerService& targetControllerService) {
        Logger::info("Handling InterruptExecution packet");

        if (targetControllerService.getTargetState() == Targets::TargetState::STOPPED) {
            debugSession.pendingInterrupt = true;
            return;
        }

        try {
            targetControllerService.stopTargetExecution();
            debugSession.connection.writePacket(TargetStopped(Signal::INTERRUPTED));
            debugSession.waitingForBreak = false;

        } catch (const Exception& exception) {
            Logger::error("Failed to interrupt execution - " + exception.getMessage());
            /*
             * We don't send an error response to GDB here, as GDB will behave as if the target is stopped. By not
             * responding, GDB will just assume the interrupt request was ignored, keeping it in-sync with Bloom.
             */
        }
    }
}
