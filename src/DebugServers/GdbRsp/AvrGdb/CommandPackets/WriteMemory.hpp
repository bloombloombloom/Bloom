#pragma once

#include <cstdint>
#include <optional>

#include "AbstractMemoryAccessPacket.hpp"

namespace Bloom::DebugServers::Gdb::AvrGdb::CommandPackets
{
    /**
     * The WriteMemory class implements the structure for "M" packets. Upon receiving this packet, the server is
     * expected to write data to the target's memory, at the specified start address.
     */
    class WriteMemory: public AbstractMemoryAccessPacket
    {
    public:
        /**
         * Like with the ReadMemory command packet, the start address carries additional bits that indicate
         * the memory type.
         */
        std::uint32_t startAddress = 0;

        Targets::TargetMemoryBuffer buffer;

        explicit WriteMemory(const std::vector<unsigned char>& rawPacket): AbstractMemoryAccessPacket(rawPacket) {
            init();
        };

        void handle(DebugSession& debugSession, TargetControllerConsole& targetControllerConsole) override;

    private:
        void init();
    };
}
