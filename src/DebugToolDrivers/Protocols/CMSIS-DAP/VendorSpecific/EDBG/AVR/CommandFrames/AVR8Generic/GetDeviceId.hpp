#pragma once

#include "Avr8GenericCommandFrame.hpp"
#include "../../ResponseFrames/AVR8Generic/GetDeviceId.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class GetDeviceId: public Avr8GenericCommandFrame
    {
    private:
        void init() {
            /*
             * The get device ID command consists of 2 bytes:
             * 1. Command ID (0x12)
             * 2. Version (0x00)
             */
            auto payload = std::vector<unsigned char>(2);
            payload[0] = 0x12;
            payload[1] = 0x00;
            this->setPayload(payload);
        }

    public:
        using ResponseFrameType = ResponseFrames::Avr8Generic::GetDeviceId;

        GetDeviceId() {
            init();
        };
    };

}
