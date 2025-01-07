#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::Avr8Generic
{
    class Step: public Avr8GenericCommandFrame<std::array<unsigned char, 4>>
    {
    public:
        Step()
            : Avr8GenericCommandFrame()
        {
            /*
             * The step command consists of 4 bytes:
             * 1. Command ID (0x34)
             * 2. Version (0x00)
             * 3. Level (0x01 for instruction level step, 0x02 for statement level step)
             * 4. Mode (0x00 for step over, 0x01 for step into, 0x02 for step out)
             */
            this->payload = {
                0x34,
                0x00,
                0x01,
                0x01,
            };
        }
    };
}
