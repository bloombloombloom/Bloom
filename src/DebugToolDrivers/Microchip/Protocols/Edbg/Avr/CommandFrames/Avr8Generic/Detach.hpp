#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::Avr8Generic
{
    class Detach: public Avr8GenericCommandFrame<std::array<unsigned char, 2>>
    {
    public:
        Detach()
            : Avr8GenericCommandFrame()
        {
            /*
             * The detach command consists of 2 bytes:
             * 1. Command ID (0x14)
             * 2. Version (0x00)
             */
            this->payload = {
                0x14,
                0x00
            };
        }
    };
}
