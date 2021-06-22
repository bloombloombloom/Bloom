#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class Step: public Avr8GenericCommandFrame
    {
    public:
        Step() = default;

        [[nodiscard]] std::vector<unsigned char> getPayload() const override {
            /*
             * The step command consists of 4 bytes:
             * 1. Command ID (0x34)
             * 2. Version (0x00)
             * 3. Level (0x01 for instruction level step, 0x02 for statement level step)
             * 4. Mode (0x00 for step over, 0x01 for step into, 0x02 for step out)
             */
            auto output = std::vector<unsigned char>(4, 0x00);
            output[0] = 0x34;
            output[1] = 0x00;
            output[2] = 0x01;
            output[3] = 0x01;

            return output;
        }
    };
}
