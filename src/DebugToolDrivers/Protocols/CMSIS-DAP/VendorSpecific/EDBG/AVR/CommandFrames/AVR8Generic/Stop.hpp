#pragma once

#include "Avr8GenericCommandFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class Stop: public Avr8GenericCommandFrame
    {
    private:
        bool stopImmediately = true;

    public:
        Stop() = default;
        Stop(bool stopImmediately) : stopImmediately(stopImmediately) {};

        void setStopImmediately(bool stopImmediately) {
            this->stopImmediately = stopImmediately;
        }

        virtual std::vector<unsigned char> getPayload() const override {
            /*
             * The stop command consists of 3 bytes:
             * 1. Command ID (0x31)
             * 2. Version (0x00)
             * 3. Stop immediately (0x01 to stop immediately or 0x02 to stop at the next symbol)
             */
            auto output = std::vector<unsigned char>(3, 0x00);
            output[0] = 0x31;
            output[1] = 0x00;
            output[2] = this->stopImmediately ? 0x01 : 0x02;

            return output;
        }
    };

}
