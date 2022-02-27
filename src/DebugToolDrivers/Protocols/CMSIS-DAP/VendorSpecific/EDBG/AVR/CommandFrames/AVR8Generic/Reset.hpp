#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class Reset: public Avr8GenericCommandFrame<std::array<unsigned char, 3>>
    {
    public:
        explicit Reset(bool stopAtMainAddress = false) {
            /*
             * The reset command consists of 3 bytes:
             * 1. Command ID (0x30)
             * 2. Version (0x00)
             * 3. "Level" (0x01 to stop at boot reset vector or 0x02 to stop at main address)
             */
            this->payload = {
                0x30,
                0x00,
                static_cast<unsigned char>(stopAtMainAddress ? 0x02 : 0x01),
            };
        }
    };
}
