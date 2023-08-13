#pragma once

#include <cstdint>

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class EraseMemory: public Avr8GenericCommandFrame<std::array<unsigned char, 7>>
    {
    public:
        EraseMemory(Avr8EraseMemoryMode mode)
            : Avr8GenericCommandFrame()
        {
            /*
             * The "Erase memory" command consists of 7 bytes:
             * 1. Command ID (0x20)
             * 2. Version (0x00)
             * 3. Erase mode (see Avr8EraseMemoryMode enum)
             * 4. Start address (4 bytes) - this is just hardcoded to 0x00000000 for now.
             */
            this->payload = {
                0x20,
                0x00,
                static_cast<unsigned char>(mode),
                0x00,
                0x00,
                0x00,
                0x00,
            };
        }
    };
}
