#pragma once

#include <cstdint>
#include <utility>

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class SetHardwareBreakpoint: public Avr8GenericCommandFrame<std::array<unsigned char, 9>>
    {
    public:
        explicit SetHardwareBreakpoint(std::uint32_t address, std::uint8_t number)
            : Avr8GenericCommandFrame()
        {
            /*
             * The set hardware breakpoint command consists of 9 bytes:
             *
             * 1. Command ID (0x40)
             * 2. Version (0x00)
             * 3. Breakpoint Type (0x01 for program break)
             * 4. Breakpoint Number (1, 2, or 3)
             * 5. Program address (4 bytes) - the EDBG Protocol document states that this should be the word address,
             *    but that seems to be incorrect. The tool expects a byte address here.
             * 5. Mode (0x03 for program break)
             */
            this->payload = {
                0x40,
                0x00,
                0x01,
                number,
                static_cast<unsigned char>(address),
                static_cast<unsigned char>(address >> 8),
                static_cast<unsigned char>(address >> 16),
                static_cast<unsigned char>(address >> 24),
                0x03
            };
        }
    };
}
