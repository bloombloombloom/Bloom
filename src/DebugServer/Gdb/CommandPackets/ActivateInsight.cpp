#include "ActivateInsight.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/EventManager/EventManager.hpp"
#include "src/EventManager/Events/InsightActivationRequested.hpp"
#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::ErrorResponsePacket;
    using Bloom::Exceptions::Exception;

    ActivateInsight::ActivateInsight(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void ActivateInsight::handle(DebugSession& debugSession, TargetControllerService&) {
        Logger::info("Handling ActivateInsight packet");

        try {
            EventManager::triggerEvent(std::make_shared<Events::InsightActivationRequested>());

            debugSession.connection.writePacket(ResponsePacket(Services::StringService::toHex(
                "Insight requested\n"
            )));

        } catch (const Exception& exception) {
            Logger::error("Failed to activate Insight - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket());
        }
    }
}
