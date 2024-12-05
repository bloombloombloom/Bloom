#pragma once

#include <cstdint>

#include "RiscVGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"
#include "src/DebugServer/Gdb/BreakpointType.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    class SetBreakpoint
        : public CommandPackets::RiscVGdbCommandPacketInterface
        , private Gdb::CommandPackets::CommandPacket
    {
    public:
        /**
         * The requested breakpoint type.
         *
         * We don't actually honor this. GDB assumes flash memory cannot be written to outside of a programming
         * session, so it refuses to even attempt to insert software breakpoints in flash memory. It always requests
         * hardware breakpoints. This is why ignore the requested breakpoint type and just insert whatever type we can.
         */
        BreakpointType type = BreakpointType::UNKNOWN;
        Targets::TargetMemoryAddress address = 0;
        Targets::TargetMemorySize size = 0;

        explicit SetBreakpoint(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
