#pragma once

#include <cstdint>
#include <optional>

#include "MemoryAccessCommandPacket.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The ReadMemory class implements a structure for "m" packets. Upon receiving these packets, the server is
     * expected to read memory from the target and send it the client.
     */
    class ReadMemory: public MemoryAccessCommandPacket
    {
    public:
        /**
         * Start address of the memory operation.
         */
        std::uint32_t startAddress = 0;

        /**
         * The type of memory to read from.
         */
        Targets::TargetMemoryType memoryType = Targets::TargetMemoryType::FLASH;

        /**
         * Number of bytes to read.
         */
        std::uint32_t bytes = 0;

        explicit ReadMemory(const RawPacketType& rawPacket);

        void handle(
            DebugSession& debugSession,
            TargetController::TargetControllerConsole& targetControllerConsole
        ) override;
    };
}
