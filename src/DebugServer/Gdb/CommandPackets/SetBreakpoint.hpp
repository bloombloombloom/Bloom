#pragma once

#include <cstdint>
#include <string>
#include <set>

#include "CommandPacket.hpp"
#include "src/DebugServer/Gdb/BreakpointType.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    /**
     * The SetBreakpoint class implements the structure for "Z" command packets. Upon receiving this command, the
     * server is expected to set a breakpoint at the specified address.
     */
    class SetBreakpoint: public CommandPacket
    {
    public:
        /**
         * Breakpoint type (Software or Hardware)
         */
        BreakpointType type = BreakpointType::UNKNOWN;

        /**
         * Address at which the breakpoint should be located.
         */
        std::uint32_t address = 0;

        explicit SetBreakpoint(const RawPacketType& rawPacket);

        void handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) override;
    };
}
