#pragma once

#include <cstdint>
#include <optional>

#include "CommandPacket.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The FlashErase class implements the structure for the "vFlashErase" packet. Upon receiving this packet, the
     * server is expected to erase a particular region of the target's flash memory.
     */
    class FlashErase: public CommandPackets::CommandPacket
    {
    public:
        std::uint32_t startAddress = 0;
        std::uint32_t bytes = 0;

        explicit FlashErase(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
