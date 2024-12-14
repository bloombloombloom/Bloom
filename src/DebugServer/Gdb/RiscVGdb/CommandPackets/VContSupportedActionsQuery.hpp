#pragma once

#include <string>
#include <set>

#include "RiscVGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    class VContSupportedActionsQuery
        : public RiscVGdbCommandPacketInterface
        , private Gdb::CommandPackets::CommandPacket
    {
    public:
        explicit VContSupportedActionsQuery(const RawPacket& rawPacket);

        void handle(
            Gdb::DebugSession& debugSession,
            const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            const Targets::TargetState& targetState,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
