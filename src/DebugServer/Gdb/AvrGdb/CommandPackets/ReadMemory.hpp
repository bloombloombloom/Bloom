#pragma once

#include <cstdint>
#include <optional>

#include "AbstractMemoryAccessPacket.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The ReadMemory class implements a structure for "m" packets. Upon receiving these packets, the server is
     * expected to read memory from the target and send it the client.
     */
    class ReadMemory: public AbstractMemoryAccessPacket
    {
    public:
        /**
         * The startAddress sent from the GDB client will include additional bits used to indicate the memory type.
         * These bits have to be removed from the address before it can be used as a start address.
         */
        std::uint32_t startAddress = 0;

        /**
         * Number of bytes to read.
         */
        std::uint32_t bytes = 0;

        explicit ReadMemory(const std::vector<unsigned char>& rawPacket);

        void handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) override;
    };
}
