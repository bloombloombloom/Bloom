#pragma once

#include "Avr8GenericCommandFrame.hpp"
#include "../../ResponseFrames/AVR8Generic/GetProgramCounter.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::Avr8Generic
{
    class GetProgramCounter: public Avr8GenericCommandFrame<std::array<unsigned char, 2>>
    {
    public:
        using ExpectedResponseFrameType = ResponseFrames::Avr8Generic::GetProgramCounter;

        GetProgramCounter()
            : Avr8GenericCommandFrame()
        {
            /*
             * The PC Read command consists of 2 bytes:
             * 1. Command ID (0x35)
             * 2. Version (0x00)
             */
            this->payload = {
                0x35,
                0x00,
            };
        }
    };
}
