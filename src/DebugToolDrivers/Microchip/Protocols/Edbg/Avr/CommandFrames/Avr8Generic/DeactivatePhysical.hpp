#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::Avr8Generic
{
    class DeactivatePhysical: public Avr8GenericCommandFrame<std::array<unsigned char, 2>>
    {
    public:
        DeactivatePhysical()
            : Avr8GenericCommandFrame()
        {
            /*
             * The deactivate physical command consists of 2 bytes:
             * 1. Command ID (0x11)
             * 2. Version (0x00)
             */
            this->payload = {
                0x11,
                0x00
            };
        }
    };
}
