#pragma once

#include <cstdint>

#include "CommandPacket.hpp"

namespace DebugServer::Gdb::CommandPackets
{
    /**
     * The VContStepExecution class implements a structure for "vCont;s" and "vCont;S" packets.
     */
    class VContStepExecution: public CommandPacket
    {
    public:
        explicit VContStepExecution(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
