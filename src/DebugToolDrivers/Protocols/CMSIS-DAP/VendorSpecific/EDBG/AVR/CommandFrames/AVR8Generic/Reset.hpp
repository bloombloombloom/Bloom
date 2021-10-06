#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class Reset: public Avr8GenericCommandFrame
    {
    public:
        Reset() = default;
        explicit Reset(bool stopAtMainAddress): stopAtMainAddress(stopAtMainAddress) {};

        void setStopAtMainAddress(bool stopAtMainAddress) {
            this->stopAtMainAddress = stopAtMainAddress;
        }

        [[nodiscard]] std::vector<unsigned char> getPayload() const override {
            /*
             * The reset command consists of 3 bytes:
             * 1. Command ID (0x30)
             * 2. Version (0x00)
             * 3. "Level" (0x01 to stop at boot reset vector or 0x02 to stop at main address)
             */
            auto output = std::vector<unsigned char>(3, 0x00);
            output[0] = 0x30;
            output[1] = 0x00;
            output[2] = this->stopAtMainAddress ? 0x02 : 0x01;

            return output;
        }

    private:
        bool stopAtMainAddress = false;
    };
}
