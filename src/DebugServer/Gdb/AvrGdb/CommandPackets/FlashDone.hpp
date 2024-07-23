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
     * The FlashDone class implements the structure for the "vFlashDone" packet.
     */
    class FlashDone: public Gdb::CommandPackets::CommandPacket
    {
    public:
        const Targets::TargetAddressSpaceDescriptor& programMemoryAddressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& programMemorySegmentDescriptor;

        explicit FlashDone(const RawPacket& rawPacket, const TargetDescriptor& targetDescriptor);

        void handle(
            Gdb::DebugSession& debugSession,
            const Gdb::TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
