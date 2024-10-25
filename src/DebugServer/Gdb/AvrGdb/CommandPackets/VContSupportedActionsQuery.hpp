#pragma once

#include <string>
#include <set>

#include "CommandPacket.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The VContSupportedActionsQuery command packet is a query from the GDB client, requesting a list of vCont actions
     * supported by the server.
     *
     * Responses to this command packet should take the form of a ResponsePackets::SupportedFeaturesResponse.
     */
    class VContSupportedActionsQuery: public CommandPackets::CommandPacket
    {
    public:
        explicit VContSupportedActionsQuery(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
