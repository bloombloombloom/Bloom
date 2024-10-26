#pragma once

#include <cstdint>

#include "AvrGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The ReadMemoryMap class implements a structure for the "qXfer:memory-map:read::..." packet. Upon receiving this
     * packet, the server is expected to respond with the target's memory map.
     */
    class ReadMemoryMap
        : public CommandPackets::AvrGdbCommandPacketInterface
        , private Gdb::CommandPackets::CommandPacket
    {
    public:
        /**
         * The offset of the memory map, from which to read.
         */
        std::uint32_t offset = 0;

        /**
         * The length of the memory map to read.
         */
        std::uint32_t length = 0;

        explicit ReadMemoryMap(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
