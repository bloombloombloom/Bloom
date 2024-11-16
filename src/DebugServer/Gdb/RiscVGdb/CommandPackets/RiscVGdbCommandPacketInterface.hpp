#pragma once

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"
#include "src/DebugServer/Gdb/DebugSession.hpp"
#include "src/DebugServer/Gdb/RiscVGdb/RiscVGdbTargetDescriptor.hpp"
#include "src/Targets/TargetDescriptor.hpp"
#include "src/Services/TargetControllerService.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    class RiscVGdbCommandPacketInterface
    {
    public:
        virtual ~RiscVGdbCommandPacketInterface() = default;

        /**
         * Should handle the command for the current active debug session.
         *
         * @param debugSession
         *  The current active debug session.
         *
         * @param TargetControllerService
         */
        virtual void handle(
            DebugSession& debugSession,
            const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) = 0;
    };
}
