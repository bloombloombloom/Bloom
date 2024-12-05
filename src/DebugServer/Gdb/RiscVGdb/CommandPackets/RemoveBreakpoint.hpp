#pragma once

#include <cstdint>

#include "RiscVGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"
#include "src/DebugServer/Gdb/BreakpointType.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    class RemoveBreakpoint
        : public CommandPackets::RiscVGdbCommandPacketInterface
        , private Gdb::CommandPackets::CommandPacket
    {
    public:
        BreakpointType type = BreakpointType::UNKNOWN;
        Targets::TargetMemoryAddress address = 0;
        Targets::TargetMemorySize size = 0;

        explicit RemoveBreakpoint(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
