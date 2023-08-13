#pragma once

#include <cstdint>

#include "Avr8GenericCommandFrame.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class RunTo: public Avr8GenericCommandFrame<std::array<unsigned char, 6>>
    {
    public:
        explicit RunTo(const std::uint32_t& address)
            : Avr8GenericCommandFrame()
        {
            /*
             * The run-to command consists of 6 bytes:
             *
             * 1. Command ID (0x33)
             * 2. Version (0x00)
             * 3. Address to run to (4 bytes)
             */

            // The address to run to needs to be the 16-bit word address, not the byte address.
            const auto wordAddress = address / 2;

            this->payload = {
                0x33,
                0x00,
                static_cast<unsigned char>(wordAddress),
                static_cast<unsigned char>(wordAddress >> 8),
                static_cast<unsigned char>(wordAddress >> 16),
                static_cast<unsigned char>(wordAddress >> 24),
            };
        }
    };
}
