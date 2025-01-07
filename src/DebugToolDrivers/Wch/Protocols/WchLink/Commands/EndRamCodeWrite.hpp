#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Wch/Protocols/WchLink/Commands/Command.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands
{
    class EndRamCodeWrite: public Command<std::array<unsigned char, 1>>
    {
    public:
        EndRamCodeWrite()
            : Command(0x02)
        {
            this->payload = {
                0x07
            };
        }
    };
}
