#pragma once

#include "CommandPacket.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    class Detach: public CommandPacket
    {
    public:
        explicit Detach(const RawPacketType& rawPacket);

        void handle(
            DebugSession& debugSession,
            TargetController::TargetControllerConsole& targetControllerConsole
        ) override;
    };
}
