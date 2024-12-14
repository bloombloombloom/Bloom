#include "VContSupportedActionsQuery.hpp"

#include "src/DebugServer/Gdb/ResponsePackets/ResponsePacket.hpp"

#include "src/Logger/Logger.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    using Services::TargetControllerService;

    VContSupportedActionsQuery::VContSupportedActionsQuery(const RawPacket& rawPacket)
        : CommandPacket(rawPacket)
    {}

    void VContSupportedActionsQuery::handle(
        Gdb::DebugSession& debugSession,
        const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
        const Targets::TargetDescriptor& targetDescriptor,
        const Targets::TargetState& targetState,
        TargetControllerService& targetControllerService
    ) {
        Logger::info("Handling VContSupportedActionsQuery packet");
        debugSession.connection.writePacket(ResponsePackets::ResponsePacket{"vCont;c;C;s;S"});
    }
}
