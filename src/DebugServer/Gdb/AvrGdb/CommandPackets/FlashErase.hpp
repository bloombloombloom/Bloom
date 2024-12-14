#pragma once

#include <cstdint>
#include <optional>

#include "AvrGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The FlashErase class implements the structure for the "vFlashErase" packet. Upon receiving this packet, the
     * server is expected to erase a particular region of the target's flash memory.
     */
    class FlashErase
        : public CommandPackets::AvrGdbCommandPacketInterface
        , private Gdb::CommandPackets::CommandPacket
    {
    public:
        std::uint32_t startAddress = 0;
        std::uint32_t bytes = 0;

        explicit FlashErase(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            const Targets::TargetState& targetState,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
