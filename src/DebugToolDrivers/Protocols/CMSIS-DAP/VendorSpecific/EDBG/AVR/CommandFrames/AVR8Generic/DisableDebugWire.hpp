#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class DisableDebugWire: public Avr8GenericCommandFrame<std::array<unsigned char, 2>>
    {
    public:
        DisableDebugWire()
            : Avr8GenericCommandFrame()
        {
            /*
             * The disable debugWire command consists of 2 bytes:
             * 1. Command ID (0x17)
             * 2. Version (0x00)
             */
            this->payload = {
                0x17,
                0x00
            };
        }
    };
}
