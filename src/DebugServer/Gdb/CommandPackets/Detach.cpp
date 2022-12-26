#include "Detach.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Helpers/Process.hpp"

#include "src/Logger/Logger.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::OkResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using Exceptions::Exception;

    Detach::Detach(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {}

    void Detach::handle(DebugSession& debugSession, TargetControllerService& targetControllerService) {
        Logger::debug("Handling Detach packet");

        try {
            if (Process::isManagedByClion()) {
                targetControllerService.suspendTargetController();
            }

            debugSession.connection.writePacket(OkResponsePacket());

        } catch (const Exception& exception) {
            Logger::error("Failed to suspend TargetController - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
