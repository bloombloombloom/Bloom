#pragma once

#include <cstdint>
#include <utility>

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class ClearHardwareBreakpoint: public Avr8GenericCommandFrame<std::array<unsigned char, 3>>
    {
    public:
        explicit ClearHardwareBreakpoint(std::uint8_t number)
            : Avr8GenericCommandFrame()
        {
            /*
             * The clear hardware breakpoint command consists of 3 bytes:
             *
             * 1. Command ID (0x41)
             * 2. Version (0x00)
             * 3. Breakpoint Number (1, 2, or 3)
             */
            this->payload = {
                0x41,
                0x00,
                number
            };
        }
    };
}
