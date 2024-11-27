#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/WCH/Protocols/WchLink/Commands/Command.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands
{
    class EndProgrammingSession: public Command<std::array<unsigned char, 1>>
    {
    public:
        EndProgrammingSession()
            : Command(0x02)
        {
            this->payload = {
                0x08
            };
        }
    };
}