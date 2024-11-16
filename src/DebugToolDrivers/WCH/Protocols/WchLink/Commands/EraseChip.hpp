#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/WCH/Protocols/WchLink/Commands/Command.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands
{
    class EraseChip: public Command<std::array<unsigned char, 1>>
    {
    public:
        EraseChip()
            : Command(0x02)
        {
            this->payload = {
                0x01
            };
        }
    };
}
