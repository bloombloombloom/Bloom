#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Wch/Protocols/WchLink/Commands/Command.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands::Control
{
    class PostAttach: public Command<std::array<unsigned char, 1>>
    {
    public:
        PostAttach()
            : Command(0x0D)
        {
            this->payload = {
                0x03
            };
        }
    };
}
