#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class Stop: public Avr8GenericCommandFrame<std::array<unsigned char, 3>>
    {
    public:
        explicit Stop(bool stopImmediately = true)
            : Avr8GenericCommandFrame()
        {
            /*
             * The stop command consists of 3 bytes:
             * 1. Command ID (0x31)
             * 2. Version (0x00)
             * 3. Stop immediately (0x01 to stop immediately or 0x02 to stop at the next symbol)
             */
            this->payload = {
                0x31,
                0x00,
                static_cast<unsigned char>(stopImmediately ? 0x01 : 0x02),
            };
        }
    };
}
