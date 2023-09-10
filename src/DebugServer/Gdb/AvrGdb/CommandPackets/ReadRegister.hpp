#pragma once

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/DebugServer/Gdb/RegisterDescriptor.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The ReadRegister class implements a structure for the "p" command packet. In response to this packet, the server
     * is expected to send register values for the requested register.
     */
    class ReadRegister: public Gdb::CommandPackets::CommandPacket
    {
    public:
        GdbRegisterId registerId;

        explicit ReadRegister(const RawPacket& rawPacket);

        void handle(
            Gdb::DebugSession& debugSession,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
