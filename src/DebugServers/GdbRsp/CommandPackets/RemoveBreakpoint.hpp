#pragma once

#include <cstdint>
#include <string>
#include <set>

#include "CommandPacket.hpp"
#include "src/DebugServers/GdbRsp/BreakpointType.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    /**
     * The RemoveBreakpoint class implements the structure for "z" command packets. Upon receiving this command, the
     * server is expected to remove a breakpoint at the specified address.
     */
    class RemoveBreakpoint: public CommandPacket
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

        explicit RemoveBreakpoint(const std::vector<unsigned char>& rawPacket): CommandPacket(rawPacket) {
            this->init();
        };

        void handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) override;

    private:
        void init();
    };
}
