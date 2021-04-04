#pragma once

#include <optional>

#include "CommandPacket.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    using namespace Bloom::DebugServers::Gdb;

    /**
     * The ReadGeneralRegisters class implements a structure for "g" and "p" command packets. In response to these
     * packets, the server is expected to send register values for all registers (for "g" packets) or for a single
     * register (for "p" packets).
     */
    class ReadGeneralRegisters: public CommandPacket
    {
    private:
        void init();

    public:
        /**
         * "p" packets include a register number to indicate which register is requested for reading. When this is set,
         * the server is expected to respond with only the value of the requested register.
         *
         * If the register number is not supplied (as is the case with "g" packets), the server is expected to respond
         * with values for all registers.
         */
        std::optional<int> registerNumber;

        ReadGeneralRegisters(std::vector<unsigned char> rawPacket) : CommandPacket(rawPacket) {
            init();
        };

        virtual void dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) override;
    };
}
