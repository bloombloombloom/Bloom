#pragma once

#include <cstdint>

#include "AvrGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"
#include "src/DebugServer/Gdb/BreakpointType.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The RemoveBreakpoint class implements the structure for "Z" command packets. Upon receiving this command, the
     * server is expected to set a breakpoint at the specified address.
     */
    class RemoveBreakpoint
        : public CommandPackets::AvrGdbCommandPacketInterface
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

        explicit RemoveBreakpoint(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            const Targets::TargetState& targetState,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
