#pragma once

#include <cstdint>
#include <optional>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"
#include "src/DebugServer/Gdb/AvrGdb/TargetDescriptor.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The FlashErase class implements the structure for the "vFlashErase" packet. Upon receiving this packet, the
     * server is expected to erase a particular region of the target's flash memory.
     */
    class FlashErase: public Gdb::CommandPackets::CommandPacket
    {
    public:
        std::uint32_t startAddress = 0;
        std::uint32_t bytes = 0;
        const Targets::TargetAddressSpaceDescriptor& programMemoryAddressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& programMemorySegmentDescriptor;

        explicit FlashErase(const RawPacket& rawPacket, const TargetDescriptor& targetDescriptor);

        void handle(
            Gdb::DebugSession& debugSession,
            const Gdb::TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
