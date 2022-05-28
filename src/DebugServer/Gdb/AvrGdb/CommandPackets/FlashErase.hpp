#pragma once

#include <cstdint>
#include <optional>

#include "MemoryAccessCommandPacket.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The FlashErase class implements the structure for the "vFlashErase" packet. Upon receiving this packet, the
     * server is expected to erase a particular region of the target's flash memory.
     */
    class FlashErase: public MemoryAccessCommandPacket
    {
    public:
        std::uint32_t startAddress = 0;
        std::uint32_t bytes = 0;

        explicit FlashErase(const RawPacketType& rawPacket);

        void handle(
            DebugSession& debugSession,
            TargetController::TargetControllerConsole& targetControllerConsole
        ) override;
    };
}
