#pragma once

#include <vector>

#include "src/DebugServer/Gdb/DebugSession.hpp"
#include "src/DebugServer/Gdb/TargetDescriptor.hpp"
#include "src/Targets/TargetDescriptor.hpp"
#include "src/Services/TargetControllerService.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    class CommandPacket
    {
    public:
        explicit CommandPacket(const RawPacket& rawPacket);

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
            const TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        );

    protected:
        std::vector<unsigned char> data;
    };
}
