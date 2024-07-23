#pragma once

#include <cstdint>

#include "src/DebugServer/Gdb/CommandPackets/Monitor.hpp"

#include "src/DebugServer/Gdb/AvrGdb/TargetDescriptor.hpp"

#include "src/Targets/TargetAddressSpaceDescriptor.hpp"
#include "src/Targets/TargetMemorySegmentDescriptor.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The EepromFill class implements a structure for the "monitor eeprom fill" GDB command.
     *
     * This command fills the target's EEPROM with the given value.
     */
    class EepromFill: public Gdb::CommandPackets::Monitor
    {
    public:
        const Targets::TargetAddressSpaceDescriptor& eepromAddressSpaceDescriptor;
        const Targets::TargetMemorySegmentDescriptor& eepromMemorySegmentDescriptor;

        explicit EepromFill(Monitor&& monitorPacket, const TargetDescriptor& gdbTargetDescriptor);

        void handle(
            Gdb::DebugSession& debugSession,
            const Gdb::TargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;

    private:
        Targets::TargetMemoryBuffer fillValue;
    };
}
