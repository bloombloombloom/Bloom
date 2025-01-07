#pragma once

#include <cstdint>

#include "src/DebugToolDrivers/Wch/Protocols/WchLink/Commands/Command.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink::Commands::Control
{
    class AttachTarget: public Command<std::array<unsigned char, 1>>
    {
    public:
        AttachTarget()
            : Command(0x0d)
        {
            this->payload = {
                0x02
            };
        }
    };
}
