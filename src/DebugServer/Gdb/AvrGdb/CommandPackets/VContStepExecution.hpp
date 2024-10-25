#pragma once

#include <cstdint>

#include "CommandPacket.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The VContStepExecution class implements a structure for "vCont;s" and "vCont;S" packets.
     */
    class VContStepExecution: public CommandPackets::CommandPacket
    {
    public:
        explicit VContStepExecution(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
