#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::CommandFrames::Avr8Generic
{
    class ActivatePhysical: public Avr8GenericCommandFrame<std::array<unsigned char, 3>>
    {
    public:
        explicit ActivatePhysical(bool reset)
            : Avr8GenericCommandFrame()
        {
            /*
             * The activate physical command consists of 3 bytes:
             * 1. Command ID (0x10)
             * 2. Version (0x00)
             * 3. Reset flag (to apply external reset)
             */
            this->payload = {
                0x10,
                0x00,
                static_cast<unsigned char>(reset)
            };
        }
    };

}
