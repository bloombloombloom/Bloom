#include "VContStepExecution.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;

    using ::Exceptions::Exception;

    VContStepExecution::VContStepExecution(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {}

    void VContStepExecution::handle(
        DebugSession& debugSession,
        const AvrGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling VContStepExecution packet");

        try {
            targetControllerService.stepTargetExecution();
            debugSession.waitingForBreak = true;

        } catch (const Exception& exception) {
            Logger::error("Failed to step execution on target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
