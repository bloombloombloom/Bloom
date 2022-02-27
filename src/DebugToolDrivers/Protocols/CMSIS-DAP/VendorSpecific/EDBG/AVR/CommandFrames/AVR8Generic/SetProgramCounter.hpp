#pragma once

#include <cstdint>

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class SetProgramCounter: public Avr8GenericCommandFrame<std::array<unsigned char, 6>>
    {
    public:
        explicit SetProgramCounter(std::uint32_t programCounter) {
            /*
             * The PC write command consists of 6 bytes:
             * 1. Command ID (0x36)
             * 2. Version (0x00)
             * 3. PC (4 bytes, LSB)
             */
            this->payload = {
                0x36,
                0x00,
                static_cast<unsigned char>(programCounter),
                static_cast<unsigned char>(programCounter >> 8),
                static_cast<unsigned char>(programCounter >> 16),
                static_cast<unsigned char>(programCounter >> 24),
            };
        }
    };
}
