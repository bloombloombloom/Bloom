#pragma once

#include "Monitor.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    class WriteRegisterBitFieldMonitor: public Monitor
    {
    public:
        explicit WriteRegisterBitFieldMonitor(Monitor&& monitorPacket);

        void handle(
            DebugSession& debugSession,
            const TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
