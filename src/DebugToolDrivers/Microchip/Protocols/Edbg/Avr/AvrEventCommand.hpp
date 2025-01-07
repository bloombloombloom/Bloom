#pragma once

#include "src/DebugToolDrivers/Protocols/CmsisDap/Command.hpp"
#include "AvrEvent.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    class AvrEventCommand: public ::DebugToolDrivers::Protocols::CmsisDap::Command
    {
    public:
        using ExpectedResponseType = AvrEvent;

        AvrEventCommand()
            : Command(0x82)
        {}
    };
}
