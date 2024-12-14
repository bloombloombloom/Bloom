#pragma once

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"
#include "src/DebugServer/Gdb/DebugSession.hpp"
#include "src/DebugServer/Gdb/AvrGdb/AvrGdbTargetDescriptor.hpp"
#include "src/Targets/TargetDescriptor.hpp"
#include "src/Targets/TargetState.hpp"
#include "src/Services/TargetControllerService.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    class AvrGdbCommandPacketInterface
    {
    public:
        virtual ~AvrGdbCommandPacketInterface() = default;

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
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            const Targets::TargetState& targetState,
            Services::TargetControllerService& targetControllerService
        ) = 0;
    };
}
