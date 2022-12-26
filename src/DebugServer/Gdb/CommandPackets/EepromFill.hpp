#pragma once

#include <cstdint>

#include "Monitor.hpp"

#include "src/Targets/TargetMemory.hpp"

namespace Bloom::DebugServer::Gdb::CommandPackets
{
    /**
     * The EepromFill class implements a structure for the "monitor eeprom fill" GDB command.
     *
     * This command fills the target's EEPROM with the given value.
     */
    class EepromFill: public Monitor
    {
    public:
        explicit EepromFill(Monitor&& monitorPacket);

        void handle(
            DebugSession& debugSession,
            Services::TargetControllerService& targetControllerService
        ) override;

    private:
        Targets::TargetMemoryBuffer fillValue;
    };
}
