#pragma once

#include <cstdint>

#include "CommandPacket.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The VContContinueExecution class implements a structure for "vCont;c" and "vCont;C" packets. These packets
     * instruct the server to continue execution on the target.
     */
    class VContContinueExecution: public CommandPackets::CommandPacket
    {
    public:
        explicit VContContinueExecution(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
