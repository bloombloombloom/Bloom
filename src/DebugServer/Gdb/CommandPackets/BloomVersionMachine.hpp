#pragma once

#include <cstdint>

#include "Monitor.hpp"

namespace DebugServer::Gdb::CommandPackets
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
            const TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
