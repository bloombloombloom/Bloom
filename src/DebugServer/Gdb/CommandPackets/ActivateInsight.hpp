#pragma once

#include "Monitor.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    /**
     * The ActivateInsight class implements a structure for the "monitor insight" GDB command.
     *
     * This command will activate the Insight GUI.
     */
    class ActivateInsight: public Monitor
    {
    public:
        explicit ActivateInsight(Monitor&& monitorPacket);

        void handle(
            DebugSession& debugSession,
            const TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
