#pragma once

#include <vector>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap
{
    class AvrEventCommand: public Command
    {
    public:
        AvrEventCommand() {
            this->setCommandId(0x82);
        }
    };
}
