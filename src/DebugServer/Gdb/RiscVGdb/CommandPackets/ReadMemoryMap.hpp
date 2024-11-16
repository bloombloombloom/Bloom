#pragma once

#include <cstdint>

#include "RiscVGdbCommandPacketInterface.hpp"
#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

namespace DebugServer::Gdb::RiscVGdb::CommandPackets
{
    class ReadMemoryMap
        : public RiscVGdbCommandPacketInterface
        , private Gdb::CommandPackets::CommandPacket
    {
    public:
        /**
         * The offset of the memory map, from which to read.
         */
        std::uint32_t offset = 0;

        /**
         * The length of the memory map to read.
         */
        std::uint32_t length = 0;

        explicit ReadMemoryMap(const RawPacket& rawPacket);

        void handle(
            Gdb::DebugSession& debugSession,
            const RiscVGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
