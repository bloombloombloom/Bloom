#pragma once

#include <cstdint>
#include <optional>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"
#include "src/DebugServer/Gdb/TargetDescriptor.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The FlashWrite class implements the structure for the "vFlashWrite" packet. Upon receiving this packet, the
     * server is expected to write to a particular region of the target's flash memory.
     */
    class FlashWrite: public Gdb::CommandPackets::CommandPacket
    {
    public:
        std::uint32_t startAddress = 0;
        Targets::TargetMemoryBuffer buffer;

        explicit FlashWrite(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
