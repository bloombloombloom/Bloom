#pragma once

#include "HouseKeepingCommandFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::HouseKeeping
{
    /**
     * The End Session command ends the active session with the tool.
     */
    class EndSession: public HouseKeepingCommandFrame<std::array<unsigned char, 3>>
    {
    public:
        EndSession()
            : HouseKeepingCommandFrame()
        {
            /*
             * The payload for the End Session command consists of three bytes. A command ID byte (0x11), a
             * version byte (0x00) and a reset flag (0x00 for no reset, 0x01 for tool reset).
             */
            this->payload = {
                0x11,
                0x00,
                0x00
            };
        }
    };
}
