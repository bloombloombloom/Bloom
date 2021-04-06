#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class Attach: public Avr8GenericCommandFrame
    {
    private:
        bool breakAfterAttach = false;

    public:
        Attach() = default;
        Attach(bool breakAfterAttach): breakAfterAttach(breakAfterAttach) {};

        void setBreadAfterAttach(bool breakAfterAttach) {
            this->breakAfterAttach = breakAfterAttach;
        }

        std::vector<unsigned char> getPayload() const override {
            /*
             * The attach command consists of 3 bytes:
             * 1. Command ID (0x13)
             * 2. Version (0x00)
             * 3. Break (stop) after attach flag
             */
            auto output = std::vector<unsigned char>(3, 0x00);
            output[0] = 0x13;
            output[1] = 0x00;
            output[2] = static_cast<unsigned char>(this->breakAfterAttach);

            return output;
        }
    };

}
