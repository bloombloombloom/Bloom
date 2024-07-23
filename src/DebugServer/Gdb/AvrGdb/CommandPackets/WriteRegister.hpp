#pragma once

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/DebugServer/Gdb/RegisterDescriptor.hpp"
#include "src/DebugServer/Gdb/AvrGdb/TargetDescriptor.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The WriteRegister class implements the structure for "P" packets.
     */
    class WriteRegister: public Gdb::CommandPackets::CommandPacket
    {
    public:
        GdbRegisterId registerId;
        Targets::TargetMemoryBuffer registerValue;

        explicit WriteRegister(const RawPacket& rawPacket);

        void handle(
            Gdb::DebugSession& debugSession,
            const Gdb::TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
