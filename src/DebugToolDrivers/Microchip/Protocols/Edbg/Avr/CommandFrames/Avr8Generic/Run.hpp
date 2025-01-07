#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::Avr8Generic
{
    class Run: public Avr8GenericCommandFrame<std::array<unsigned char, 2>>
    {
    public:
        Run()
            : Avr8GenericCommandFrame()
        {
            /*
             * The run command consists of 2 bytes:
             * 1. Command ID (0x32)
             * 2. Version (0x00)
             */
            this->payload = {
                0x32,
                0x00,
            };
        }
    };
}
