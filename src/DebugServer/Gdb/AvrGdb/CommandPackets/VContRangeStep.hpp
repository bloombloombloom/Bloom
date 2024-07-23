#pragma once

#include <cstdint>

#include "src/DebugServer/Gdb/CommandPackets/CommandPacket.hpp"

#include "src/DebugServer/Gdb/AvrGdb/TargetDescriptor.hpp"
#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"
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
        const Targets::TargetAddressSpaceDescriptor& programAddressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& programMemorySegmentDescriptor;

        Targets::TargetMemoryAddress startAddress;
        Targets::TargetMemoryAddress endAddress;

        explicit VContRangeStep(const RawPacket& rawPacket, const TargetDescriptor& gdbTargetDescriptor);

        void handle(
            Gdb::DebugSession& debugSession,
            const Gdb::TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
