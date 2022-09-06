#pragma once

#include <cstdint>
#include <optional>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"
#include "src/DebugServer/Gdb/TargetDescriptor.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The ReadMemory class implements a structure for "m" packets. Upon receiving these packets, the server is
     * expected to read memory from the target and send it the client.
     */
    class ReadMemory: public Gdb::CommandPackets::CommandPacket
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
         * Number of bytes to read.
         */
        Targets::TargetMemorySize bytes = 0;

        explicit ReadMemory(const RawPacketType& rawPacket, const Gdb::TargetDescriptor& gdbTargetDescriptor);

        void handle(
            DebugSession& debugSession,
            TargetController::TargetControllerConsole& targetControllerConsole
        ) override;
    };
}
