#include "VContContinueExecution.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;

    using ::Exceptions::Exception;

    VContContinueExecution::VContContinueExecution(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {}

    void VContContinueExecution::handle(
        DebugSession& debugSession,
        const AvrGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling VContContinueExecution packet");

        try {
            targetControllerService.resumeTargetExecution();
            debugSession.waitingForBreak = true;

        } catch (const Exception& exception) {
            Logger::error("Failed to continue execution on target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
