#pragma once

#include <cstdint>

#include "Monitor.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    /**
     * The ResetTarget class implements a structure for the custom reset command (triggered via the "monitor reset"
     * GDB command
     *
     * The "monitor reset" command will trigger a target reset and hold the target in a stopped state.
     */
    class ResetTarget: public Monitor
    {
    public:
        explicit ResetTarget(Monitor&& monitorPacket);

        void handle(
            DebugSession& debugSession,
            const TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
