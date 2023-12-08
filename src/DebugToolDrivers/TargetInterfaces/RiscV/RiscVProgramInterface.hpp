#pragma once

#include <cstdint>

#include "src/Targets/TargetMemory.hpp"

namespace DebugToolDrivers::TargetInterfaces::RiscV
{
    class RiscVProgramInterface
    {
    public:
        /**
         * Should write to the target's FLASH memory.
         *
         * @param startAddress
         * @param buffer
         */
        virtual void writeFlashMemory(
            Targets::TargetMemoryAddress startAddress,
            const Targets::TargetMemoryBuffer& buffer
        ) = 0;
    };
}
