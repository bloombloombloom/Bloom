#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class ActivatePhysical: public Avr8GenericCommandFrame
    {
    public:
        ActivatePhysical() = default;
        explicit ActivatePhysical(bool reset): reset(reset) {};

        void setReset(bool reset) {
            this->reset = reset;
        }

        [[nodiscard]] std::vector<unsigned char> getPayload() const override {
            /*
             * The activate physical command consists of 3 bytes:
             * 1. Command ID (0x10)
             * 2. Version (0x00)
             * 3. Reset flag (to apply external reset)
             */
            auto output = std::vector<unsigned char>(3, 0x00);
            output[0] = 0x10;
            output[1] = 0x00;
            output[2] = static_cast<unsigned char>(this->reset);

            return output;
        }

    private:
        bool reset = false;
    };

}
