#pragma once

#include <cstdint>

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class SetXmegaSoftwareBreakpoint: public Avr8GenericCommandFrame<std::array<unsigned char, 15>>
    {
    public:
        explicit SetXmegaSoftwareBreakpoint(std::uint32_t address) {
            this->payload = {
                0x42,
                0x00,
                static_cast<unsigned char>(address),
                static_cast<unsigned char>(address >> 8),
                static_cast<unsigned char>(address >> 16),
                static_cast<unsigned char>(address >> 24),
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x00,
                0x01,
                0x00,
            };
        }
    };
}
