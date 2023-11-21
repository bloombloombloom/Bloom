#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/WCH/Protocols/WchLink/Commands/Command.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands
{
    class SetClockSpeed: public Command<std::array<unsigned char, 2>>
    {
    public:
        SetClockSpeed(std::uint8_t targetGroupId, std::uint8_t speedId)
            : Command(0x0c)
        {
            this->payload = {
                targetGroupId,
                speedId,
            };
        }
    };
}
