#pragma once

#include <string>
#include <set>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The VContSupportedActionsQuery command packet is a query from the GDB client, requesting a list of vCont actions
     * supported by the server.
     *
     * Responses to this command packet should take the form of a ResponsePackets::SupportedFeaturesResponse.
     */
    class VContSupportedActionsQuery: public Gdb::CommandPackets::CommandPacket
    {
    public:
        explicit VContSupportedActionsQuery(const RawPacket& rawPacket);

        void handle(
            Gdb::DebugSession& debugSession,
            const Gdb::TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
