#pragma once

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"
#include "src/DebugServer/Gdb/DebugSession.hpp"
#include "src/DebugServer/Gdb/RiscVGdb/RiscVGdbTargetDescriptor.hpp"
#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetState.hpp"
#include "src/Services/TargetControllerService.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    class RiscVGdbCommandPacketInterface
    {
    public:
        virtual ~RiscVGdbCommandPacketInterface() = default;

        virtual void handle(
            DebugSession& debugSession,
            const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            const Targets::TargetState& targetState,
            Services::TargetControllerService& targetControllerService
        ) = 0;
    };
}
