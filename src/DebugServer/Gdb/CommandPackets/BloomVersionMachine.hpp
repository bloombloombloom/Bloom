#pragma once

#include <cstdint>

#include "Monitor.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    /**
     * The BloomVersionMachine class implements a structure for the "monitor version machine" GDB command.
     *
     * We just output Bloom's current version number in JSON format.
     */
    class BloomVersionMachine: public Monitor
    {
    public:
        explicit BloomVersionMachine(Monitor&& monitorPacket);

        void handle(
            DebugSession& debugSession,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
