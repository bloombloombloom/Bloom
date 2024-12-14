#pragma once

#include <cstdint>
#include <optional>

#include "RiscVGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    class FlashErase
        : public RiscVGdbCommandPacketInterface
        , private Gdb::CommandPackets::CommandPacket
    {
    public:
        std::uint32_t startAddress = 0;
        std::uint32_t bytes = 0;

        explicit FlashErase(const RawPacket& rawPacket);

        void handle(
            Gdb::DebugSession& debugSession,
            const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            const Targets::TargetState& targetState,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
