#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class ClearAllSoftwareBreakpoints: public Avr8GenericCommandFrame<std::array<unsigned char, 2>>
    {
    public:
        ClearAllSoftwareBreakpoints()
            : Avr8GenericCommandFrame()
        {
            /*
             * The clear all software breakpoints command consists of 2 bytes:
             * 1. Command ID (0x45)
             * 2. Version (0x00)
             */
            this->payload = {
                0x45,
                0x00,
            };
        }
    };
}
