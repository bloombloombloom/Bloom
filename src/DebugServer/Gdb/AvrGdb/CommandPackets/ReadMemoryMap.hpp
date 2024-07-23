#pragma once

#include <cstdint>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"
#include "src/DebugServer/Gdb/AvrGdb/TargetDescriptor.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The ReadMemoryMap class implements a structure for the "qXfer:memory-map:read::..." packet. Upon receiving this
     * packet, the server is expected to respond with the target's memory map.
     */
    class ReadMemoryMap: public Gdb::CommandPackets::CommandPacket
    {
    public:
        const Targets::TargetAddressSpaceDescriptor& eepromAddressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& programMemorySegmentDescriptor;
        const Targets::TargetMemorySegmentDescriptor& eepromMemorySegmentDescriptor;

        /**
         * The offset of the memory map, from which to read.
         */
        std::uint32_t offset = 0;

        /**
         * The length of the memory map to read.
         */
        std::uint32_t length = 0;

        explicit ReadMemoryMap(const RawPacket& rawPacket, const TargetDescriptor& gdbTargetDescriptor);

        void handle(
            Gdb::DebugSession& debugSession,
            const Gdb::TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
