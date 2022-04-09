#pragma once

#include <cstdint>
#include <optional>

#include "MemoryAccessCommandPacket.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The WriteMemory class implements the structure for "M" packets. Upon receiving this packet, the server is
     * expected to write data to the target's memory, at the specified start address.
     */
    class WriteMemory: public MemoryAccessCommandPacket
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
         * Data to write.
         */
        Targets::TargetMemoryBuffer buffer;

        explicit WriteMemory(const RawPacketType& rawPacket);

        void handle(
            DebugSession& debugSession,
            TargetController::TargetControllerConsole& targetControllerConsole
        ) override;
    };
}
