#pragma once

#include "CommandPacket.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    class Detach: public CommandPacket
    {
    public:
        explicit Detach(const RawPacket& rawPacket);

        bool requiresBreakpointFlush() const override {
            return true;
        }

        void handle(
            DebugSession& debugSession,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
