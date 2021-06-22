#pragma once

#include <cstdint>

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class SetProgramCounter: public Avr8GenericCommandFrame
    {
    private:
        std::uint32_t programCounter = 0;

    public:
        explicit SetProgramCounter(std::uint32_t programCounter): programCounter(programCounter) {}

        [[nodiscard]] std::vector<unsigned char> getPayload() const override {
            /*
             * The PC write command consists of 6 bytes:
             * 1. Command ID (0x01)
             * 2. Version (0x00)
             * 3. PC (4 bytes, LSB)
             */
            auto output = std::vector<unsigned char>(6, 0x00);
            output[0] = 0x36;
            output[1] = 0x00;
            output[2] = static_cast<unsigned char>(this->programCounter);
            output[3] = static_cast<unsigned char>(this->programCounter >> 8);
            output[4] = static_cast<unsigned char>(this->programCounter >> 16);
            output[5] = static_cast<unsigned char>(this->programCounter >> 24);

            return output;
        }
    };
}
