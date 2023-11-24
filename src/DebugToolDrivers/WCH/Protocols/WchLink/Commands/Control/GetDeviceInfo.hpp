#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/WCH/Protocols/WchLink/Commands/Command.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands::Control
{
    class GetDeviceInfo: public Command<std::array<unsigned char, 1>>
    {
    public:
        GetDeviceInfo()
            : Command(0x0d)
        {
            this->payload = {
                0x01
            };
        }
    };
}