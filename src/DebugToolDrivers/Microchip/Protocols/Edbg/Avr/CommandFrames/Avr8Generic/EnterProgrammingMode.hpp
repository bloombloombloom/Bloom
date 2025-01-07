#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::Avr8Generic
{
    class EnterProgrammingMode: public Avr8GenericCommandFrame<std::array<unsigned char, 2>>
    {
    public:
        EnterProgrammingMode()
            : Avr8GenericCommandFrame()
        {
            /*
             * The enter programming mode command consists of 2 bytes:
             * 1. Command ID (0x15)
             * 2. Version (0x00)
             */
            this->payload = {
                0x15,
                0x00
            };
        }
    };
}
