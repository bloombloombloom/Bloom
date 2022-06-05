#pragma once

#include <cstdint>
#include <optional>

#include "MemoryAccessCommandPacket.hpp"

namespace Bloom::DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The FlashDone class implements the structure for the "vFlashDone" packet.
     */
    class FlashDone: public MemoryAccessCommandPacket
    {
    public:
        explicit FlashDone(const RawPacketType& rawPacket);

        void handle(
            DebugSession& debugSession,
            TargetController::TargetControllerConsole& targetControllerConsole
        ) override;
    };
}
