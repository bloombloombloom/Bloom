#pragma once

#include <optional>

#include "CommandPacket.hpp"

#include "src/DebugServer/Gdb/RegisterDescriptor.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The ReadRegisters class implements a structure for the "g" command packet. In response to this packet, the
     * server is expected to send register values for all registers.
     */
    class ReadRegisters: public CommandPackets::CommandPacket
    {
    public:
        explicit ReadRegisters(const RawPacket& rawPacket);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
