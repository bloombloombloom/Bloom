#pragma once

#include <cstdint>
#include <optional>

#include "CommandPacket.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::DebugServers::Gdb::CommandPackets
{
    /**
     * The WriteMemory class implements the structure for "M" packets. Upon receiving this packet, the server is
     * expected to write data to the target's memory, at the specified start address.
     */
    class WriteMemory: public CommandPacket
    {
    private:
        void init();

    public:
        /**
         * Like with the ReadMemory command packet, the start address carries additional bits that indicate
         * the memory type.
         */
        std::uint32_t startAddress;

        Targets::TargetMemoryBuffer buffer;

        WriteMemory(std::vector<unsigned char> rawPacket): CommandPacket(rawPacket) {
            init();
        };

        virtual void dispatchToHandler(Gdb::GdbRspDebugServer& gdbRspDebugServer) override;
    };
}
