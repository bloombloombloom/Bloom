#pragma once

#include "Monitor.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    class WriteRegisterMonitor: public Monitor
    {
    public:
        explicit WriteRegisterMonitor(Monitor&& monitorPacket);

        void handle(
            DebugSession& debugSession,
            const TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
