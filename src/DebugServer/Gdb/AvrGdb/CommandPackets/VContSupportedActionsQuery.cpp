#include "VContSupportedActionsQuery.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Logger/Logger.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    using Services::TargetControllerService;

    VContSupportedActionsQuery::VContSupportedActionsQuery(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {}

    void VContSupportedActionsQuery::handle(Gdb::DebugSession& debugSession, TargetControllerService& targetControllerService) {
        Logger::info("Handling VContSupportedActionsQuery packet");

        // Respond with a SupportedFeaturesResponse packet, listing all supported GDB features by Bloom
        debugSession.connection.writePacket(ResponsePackets::ResponsePacket(
            debugSession.serverConfig.rangeSteppingEnabled
                ? "vCont;c;C;s;S;r"
                : "vCont;c;C;s;S"
            )
        );
    }
}
