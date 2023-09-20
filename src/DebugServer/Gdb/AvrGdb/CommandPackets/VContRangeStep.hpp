#pragma once

#include <cstdint>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The VContRangeStep class implements a structure for the "vCont;r" packet. This packet instructs the server to
     * step through a particular address range, and only report back to GDB when execution leaves that range, or when an
     * external breakpoint has been reached.
     */
    class VContRangeStep: public Gdb::CommandPackets::CommandPacket
    {
    public:
        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemoryAddress endAddress;

        explicit VContRangeStep(const RawPacket& rawPacket);

        void handle(
            Gdb::DebugSession& debugSession,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
