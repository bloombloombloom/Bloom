#pragma once

#include <cstdint>
#include <optional>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"
#include "src/DebugServer/Gdb/TargetDescriptor.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The FlashDone class implements the structure for the "vFlashDone" packet.
     */
    class FlashDone: public Gdb::CommandPackets::CommandPacket
    {
    public:
        explicit FlashDone(const RawPacket& rawPacket);

        void handle(
            Gdb::DebugSession& debugSession,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
