#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class Attach: public Avr8GenericCommandFrame<std::array<unsigned char, 3>>
    {
    public:
        explicit Attach(bool breakAfterAttach)
            : Avr8GenericCommandFrame()
        {
            /*
             * The attach command consists of 3 bytes:
             * 1. Command ID (0x13)
             * 2. Version (0x00)
             * 3. Break (stop) after attach flag
             */
            this->payload = {
                0x13,
                0x00,
                static_cast<unsigned char>(breakAfterAttach)
            };
        }
    };
}
