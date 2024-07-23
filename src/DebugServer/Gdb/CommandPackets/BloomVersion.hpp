#pragma once

#include <cstdint>

#include "Monitor.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    /**
     * The BloomVersion class implements a structure for the "monitor version" GDB command.
     *
     * We just output Bloom's current version number.
     */
    class BloomVersion: public Monitor
    {
    public:
        explicit BloomVersion(Monitor&& monitorPacket);

        void handle(
            DebugSession& debugSession,
            const TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
