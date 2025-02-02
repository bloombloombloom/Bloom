#include "ResetTarget.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ErrorResponsePacket;
    using ResponsePackets::ResponsePacket;

    using ::Exceptions::Exception;

    ResetTarget::ResetTarget(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void ResetTarget::handle(
        DebugSession& debugSession,
        const TargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor&,
        TargetControllerService& targetControllerService
    ) {
        using Services::StringService;

        Logger::info("Handling ResetTarget packet");

        try {
            Logger::warning("Resetting target");
            targetControllerService.resetTarget();
            Logger::info("Target reset complete");

            debugSession.connection.writePacket(ResponsePacket{Services::StringService::toHex(
                "Target reset complete\n"
                "Current PC: 0x" + StringService::asciiToUpper(
                    StringService::toHex(targetControllerService.getProgramCounter())
                ) + "\nUse the 'continue' command to begin execution\n"
            )});

        } catch (const Exception& exception) {
            Logger::error("Failed to reset target - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
