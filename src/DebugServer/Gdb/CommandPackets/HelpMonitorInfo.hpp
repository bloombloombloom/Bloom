#pragma once

#include <cstdint>

#include "Monitor.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    /**
     * The HelpMonitorInfo class implements a structure for the "monitor help" GDB command.
     *
     * We just respond with some help info on the available "monitor" commands.
     */
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
