#pragma once

#include <cstdint>
#include <string>
#include <set>

#include "../BreakpointType.hpp"
#include "CommandPacket.hpp"

namespace Bloom::DebugServers::Gdb
{
    enum class Feature: int;
}

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

        void dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) override;

    private:
        void init();
    };
}
