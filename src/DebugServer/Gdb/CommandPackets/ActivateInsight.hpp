#pragma once

#include "Monitor.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
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
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
