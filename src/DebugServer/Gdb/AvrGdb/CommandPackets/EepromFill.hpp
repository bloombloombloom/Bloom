#pragma once

#include <cstdint>
#include <string>

#include "CommandPacket.hpp"

#include "src/DebugServer/Gdb/CommandPackets/Monitor.hpp"

namespace DebugServer::Gdb::AvrGdb::CommandPackets
{
    /**
     * The EepromFill class implements a structure for the "monitor eeprom fill" GDB command.
     *
     * This command fills the target's EEPROM with the given value.
     */
    class EepromFill: public CommandPackets::CommandPacket
    {
    public:
        std::string rawFillValue;

        explicit EepromFill(Gdb::CommandPackets::Monitor&& monitorPacket);

        void handle(
            DebugSession& debugSession,
            const AvrGdbTargetDescriptor& gdbTargetDescriptor,
            const Targets::TargetDescriptor& targetDescriptor,
            Services::TargetControllerService& targetControllerService
        ) override;
    };
}
