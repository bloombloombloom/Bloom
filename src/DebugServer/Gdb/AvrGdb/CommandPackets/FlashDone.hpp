#pragma once

#include <cstdint>
#include <optional>

#include "CommandPacket.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The FlashDone class implements the structure for the "vFlashDone" packet.
     */
    class FlashDone: public CommandPackets::CommandPacket
    {
    public:
        explicit FlashDone(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
