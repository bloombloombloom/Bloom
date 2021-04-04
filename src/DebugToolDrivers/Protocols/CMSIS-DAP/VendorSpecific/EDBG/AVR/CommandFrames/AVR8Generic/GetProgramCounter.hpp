#pragma once

#include "Avr8GenericCommandFrame.hpp"
#include "../../ResponseFrames/AVR8Generic/GetProgramCounter.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class GetProgramCounter: public Avr8GenericCommandFrame
    {
    private:
        void init() {
            /*
             * The PC Read command consists of 2 bytes:
             * 1. Command ID (0x35)
             * 2. Version (0x00)
             */
            auto payload = std::vector<unsigned char>(2);
            payload[0] = 0x35;
            payload[1] = 0x00;
            this->setPayload(payload);
        }

    public:
        using ResponseFrameType = ResponseFrames::Avr8Generic::GetProgramCounter;

        GetProgramCounter() {
            init();
        };
    };

}
