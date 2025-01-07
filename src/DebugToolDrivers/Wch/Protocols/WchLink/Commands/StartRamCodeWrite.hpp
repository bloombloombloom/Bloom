#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Wch/Protocols/WchLink/Commands/Command.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands
{
    class StartRamCodeWrite: public Command<std::array<unsigned char, 1>>
    {
    public:
        StartRamCodeWrite()
            : Command(0x02)
        {
            this->payload = {
                0x05
            };
        }
    };
}
