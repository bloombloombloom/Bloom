#pragma once

#include "CommandPacket.hpp"

#include "src/DebugServer/Gdb/RegisterDescriptor.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The WriteRegister class implements the structure for "P" packets.
     */
    class WriteRegister: public CommandPackets::CommandPacket
    {
    public:
        GdbRegisterId registerId;
        Targets::TargetMemoryBuffer registerValue;

        explicit WriteRegister(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
