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
     * The SetBreakpoint class implements the structure for "Z" command packets. Upon receiving this command, the
     * server is expected to set a breakpoint at the specified address.
     */
    class SetBreakpoint: public CommandPacket
    {
    private:
        void init();

    public:
        /**
         * Breakpoint type (Software or Hardware)
         */
        BreakpointType type = BreakpointType::UNKNOWN;

        /**
         * Address at which the breakpoint should be located.
         */
        std::uint32_t address = 0;

        explicit SetBreakpoint(const std::vector<unsigned char>& rawPacket): CommandPacket(rawPacket) {
            this->init();
        };

        void dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) override;
    };
}
