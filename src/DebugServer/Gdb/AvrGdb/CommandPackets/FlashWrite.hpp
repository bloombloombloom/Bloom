#pragma once

#include <cstdint>
#include <optional>

#include "AvrGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The FlashWrite class implements the structure for the "vFlashWrite" packet. Upon receiving this packet, the
     * server is expected to write to a particular region of the target's flash memory.
     */
    class FlashWrite
        : public CommandPackets::AvrGdbCommandPacketInterface
        , private Gdb::CommandPackets::CommandPacket
    {
    public:
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemoryBuffer buffer;

        explicit FlashWrite(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            const Targets::TargetState& targetState,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
