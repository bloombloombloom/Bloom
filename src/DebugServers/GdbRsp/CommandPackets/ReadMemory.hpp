#pragma once

#include <cstdint>
#include <optional>

#include "CommandPacket.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    using namespace Bloom::DebugServers::Gdb;

    /**
     * The ReadMemory class implements a structure for "m" packets. Upon receiving these packets, the server is
     * expected to read memory from the target and send it the client.
     */
    class ReadMemory: public CommandPacket
    {
    private:
        void init();

    public:
        /**
         * The startAddress sent from the GDB client may include additional bits used to indicate the memory type.
         * These bits have to be removed from the address before it can be used as a start address. This is not done
         * here, as it's target specific.
         *
         * For an example of where GDB does this, see the AvrGdbRsp class.
         */
        std::uint32_t startAddress;

        /**
         * Number of bytes to read.
         */
        std::uint32_t bytes;

        ReadMemory(std::vector<unsigned char> rawPacket): CommandPacket(rawPacket) {
            init();
        };

        virtual void dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) override;
    };
}
