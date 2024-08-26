#pragma once

#include <cstdint>

#include "Monitor.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    class HelpMonitorInfo: public Monitor
    {
    public:
        explicit HelpMonitorInfo(Monitor&& monitorPacket);

        void handle(
            DebugSession& debugSession,
            const TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
