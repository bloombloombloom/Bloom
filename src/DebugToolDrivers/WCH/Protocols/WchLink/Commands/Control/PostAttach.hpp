#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/WCH/Protocols/WchLink/Commands/Command.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands::Control
{
    class PostAttach: public Command<std::array<unsigned char, 1>>
    {
    public:
        PostAttach()
            : Command(0x0d)
        {
            this->payload = {
                0x03
            };
        }
    };
}
