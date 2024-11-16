#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/WCH/Protocols/WchLink/Commands/Command.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands
{
    /*
     * We're not actually using this ATM. Sending it Issuing this command before flashing the target seems to
     * result in failures. See the comment in WchLinkInterface::writeFullPage() for more.
     *
     * TODO: Consider removing at a later date.
     */
    class StartProgrammingSession: public Command<std::array<unsigned char, 1>>
    {
    public:
        StartProgrammingSession()
            : Command(0x02)
        {
            this->payload = {
                0x06
            };
        }
    };
}
