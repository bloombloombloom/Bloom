#pragma once

#include <cstdint>
#include <optional>

#include "AvrGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The FlashDone class implements the structure for the "vFlashDone" packet.
     */
    class FlashDone
        : public CommandPackets::AvrGdbCommandPacketInterface
        , private Gdb::CommandPackets::CommandPacket
    {
    public:
        explicit FlashDone(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            const Targets::TargetState& targetState,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
