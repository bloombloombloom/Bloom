#pragma once

#include <cstdint>
#include <optional>

#include "CommandPacket.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    /**
     * The ReadMemory class implements a structure for "m" packets. Upon receiving these packets, the server is
     * expected to read memory from the target and send it the client.
     */
    class ReadMemory: public CommandPacket
    {
    public:
        /**
         * The startAddress sent from the GDB client may include additional bits used to indicate the memory type.
         * These bits have to be removed from the address before it can be used as a start address. This is not done
         * here, as it's target specific.
         *
         * For an example of where GDB does this, see the AvrGdbRsp class.
         */
        std::uint32_t startAddress = 0;

        /**
         * Number of bytes to read.
         */
        std::uint32_t bytes = 0;

        explicit ReadMemory(const std::vector<unsigned char>& rawPacket): CommandPacket(rawPacket) {
            init();
        };

        void dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) override;

    private:
        void init();
    };
}
