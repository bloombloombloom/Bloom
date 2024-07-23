#pragma once

#include <optional>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/DebugServer/Gdb/RegisterDescriptor.hpp"
#include "src/DebugServer/Gdb/AvrGdb/TargetDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The ReadRegisters class implements a structure for the "g" command packet. In response to this packet, the
     * server is expected to send register values for all registers.
     */
    class ReadRegisters: public Gdb::CommandPackets::CommandPacket
    {
    public:
        const Targets::TargetMemorySegmentDescriptor& gpRegistersMemorySegmentDescriptor;

        explicit ReadRegisters(const RawPacket& rawPacket, const TargetDescriptor& gdbTargetDescriptor);

        void handle(
            Gdb::DebugSession& debugSession,
            const Gdb::TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
