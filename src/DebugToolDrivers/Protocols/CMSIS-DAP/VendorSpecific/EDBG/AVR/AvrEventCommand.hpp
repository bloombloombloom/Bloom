#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"
#include "AvrEvent.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    class AvrEventCommand: public Command
    {
    public:
        using ExpectedResponseType = AvrEvent;

        AvrEventCommand()
            : Command(0x82)
        {}
    };
}
