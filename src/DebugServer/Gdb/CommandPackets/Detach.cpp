#include "Detach.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/OkResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/Services/ProcessService.hpp"

#include "src/Logger/Logger.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::OkResponsePacket;
    using ResponsePackets::ErrorResponsePacket;

    using ::Exceptions::Exception;

    Detach::Detach(const RawPacket& rawPacket, const EnvironmentConfig& environmentConfig)
        : CommandPacket(rawPacket)
        , environmentConfig(environmentConfig)
    {}

    void Detach::handle(
        DebugSession& debugSession,
        const TargetDescriptor&,
        const Targets::TargetDescriptor&,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling Detach packet");

        try {
            if (
                debugSession.serverConfig.shutdownOnDetach
                || (this->environmentConfig.clionAdaptation && Services::ProcessService::isManagedByClion())
            ) {
                targetControllerService.shutdown();
            }

            debugSession.connection.writePacket(OkResponsePacket{});

        } catch (const Exception& exception) {
            Logger::error("Failed to shut down TargetController - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
