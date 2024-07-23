#include "ActivateInsight.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"
#include "src/DebugServer/Gdb/ResponsePackets/ErrorResponsePacket.hpp"

#include "src/EventManager/EventManager.hpp"
#include "src/EventManager/Events/InsightActivationRequested.hpp"
#include "src/Services/StringService.hpp"
#include "src/Logger/Logger.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    using Services::TargetControllerService;

    using ResponsePackets::ResponsePacket;
    using ResponsePackets::ErrorResponsePacket;
    using ::Exceptions::Exception;

    ActivateInsight::ActivateInsight(Monitor&& monitorPacket)
        : Monitor(std::move(monitorPacket))
    {}

    void ActivateInsight::handle(
        DebugSession& debugSession,
        const TargetDescriptor&,
        const Targets::TargetDescriptor&,
        TargetControllerService&
    ) {
        Logger::info("Handling ActivateInsight packet");

        try {
            EventManager::triggerEvent(std::make_shared<Events::InsightActivationRequested>());

            debugSession.connection.writePacket(
                ResponsePacket{Services::StringService::toHex("The Insight GUI will be with you shortly.\n")}
            );

        } catch (const Exception& exception) {
            Logger::error("Failed to activate Insight - " + exception.getMessage());
            debugSession.connection.writePacket(ErrorResponsePacket{});
        }
    }
}
