#pragma once

#include <cstdint>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The VContStepExecution class implements a structure for "vCont;s" and "vCont;S" packets.
     */
    class VContStepExecution: public Gdb::CommandPackets::CommandPacket
    {
    public:
        explicit VContStepExecution(const RawPacket& rawPacket);

        void handle(
            Gdb::DebugSession& debugSession,
            const Gdb::TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
