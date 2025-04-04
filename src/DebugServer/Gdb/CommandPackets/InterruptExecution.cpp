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

    void InterruptExecution::handle(
        DebugSession& debugSession,
        const TargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor&,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling InterruptExecution packet");

        if (targetControllerService.getTargetState().executionState == Targets::TargetExecutionState::STOPPED) {
            debugSession.pendingInterrupt = true;
            return;
        }

        try {
            targetControllerService.stopTargetExecution();

            if (debugSession.activeRangeSteppingSession.has_value()) {
                debugSession.terminateRangeSteppingSession(targetControllerService);
            }

            debugSession.waitingForBreak = false;
            debugSession.connection.writePacket(TargetStopped{Signal::INTERRUPTED});

        } catch (const Exception& exception) {
            Logger::error("Failed to interrupt execution - " + exception.getMessage());
            /*
             * We don't send an error response to GDB here, as GDB will behave as if the target is stopped. By not
             * responding, GDB will just assume the interrupt request was ignored, keeping it in-sync with Bloom.
             */
        }
    }
}
