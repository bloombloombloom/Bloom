#pragma once

#include <cstdint>
#include <optional>

#include "MemoryAccessCommandPacket.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The FlashWrite class implements the structure for the "vFlashWrite" packet. Upon receiving this packet, the
     * server is expected to write to a particular region of the target's flash memory.
     */
    class FlashWrite: public MemoryAccessCommandPacket
    {
    public:
        std::uint32_t startAddress = 0;
        Targets::TargetMemoryBuffer buffer;

        explicit FlashWrite(const RawPacketType& rawPacket);

        void handle(
            DebugSession& debugSession,
            TargetController::TargetControllerConsole& targetControllerConsole
        ) override;
    };
}
