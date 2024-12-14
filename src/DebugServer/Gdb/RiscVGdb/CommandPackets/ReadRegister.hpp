#pragma once

#include "RiscVGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/DebugServer/Gdb/RegisterDescriptor.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    class ReadRegister
        : public RiscVGdbCommandPacketInterface
        , private Gdb::CommandPackets::CommandPacket
    {
    public:
        GdbRegisterId registerId;

        explicit ReadRegister(const RawPacket& rawPacket);

        void handle(
            Gdb::DebugSession& debugSession,
            const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            const Targets::TargetState& targetState,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
