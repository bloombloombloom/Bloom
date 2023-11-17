#pragma once

#include <array>

#include "HouseKeepingCommandFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::HouseKeeping
{
    /**
     * The Start Session command begins a session with the tool.
     */
    class StartSession: public HouseKeepingCommandFrame<std::array<unsigned char, 2>>
    {
    public:
        StartSession()
            : HouseKeepingCommandFrame()
        {
            /*
             * The payload for the Start Session command consists of two bytes. A command ID byte (0x10) and a
             * version byte (0x00).
             */
            this->payload = {
                0x10,
                0x00
            };
        }
    };
}
