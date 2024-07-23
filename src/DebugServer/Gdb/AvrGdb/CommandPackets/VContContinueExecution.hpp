#pragma once

#include <cstdint>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The VContContinueExecution class implements a structure for "vCont;c" and "vCont;C" packets. These packets
     * instruct the server to continue execution on the target.
     */
    class VContContinueExecution: public Gdb::CommandPackets::CommandPacket
    {
    public:
        explicit VContContinueExecution(const RawPacket& rawPacket);

        void handle(
            Gdb::DebugSession& debugSession,
            const Gdb::TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
