#pragma once

#include <cstdint>
#include <optional>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"
#include "src/DebugServer/Gdb/TargetDescriptor.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The WriteMemory class implements the structure for "M" packets. Upon receiving this packet, the server is
     * expected to write data to the target's memory, at the specified start address.
     */
    class WriteMemory: public Gdb::CommandPackets::CommandPacket
    {
    public:
        /**
         * Start address of the memory operation.
         */
        Targets::TargetMemoryAddress startAddress = 0;

        /**
         * The type of memory to read from.
         */
        Targets::TargetMemoryType memoryType = Targets::TargetMemoryType::FLASH;

        /**
         * Data to write.
         */
        Targets::TargetMemoryBuffer buffer;

        explicit WriteMemory(const RawPacket& rawPacket, const Gdb::TargetDescriptor& gdbTargetDescriptor);

        void handle(
            DebugSession& debugSession,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
