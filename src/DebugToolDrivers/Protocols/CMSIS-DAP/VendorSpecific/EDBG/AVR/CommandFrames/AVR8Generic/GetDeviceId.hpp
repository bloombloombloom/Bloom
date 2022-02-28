#pragma once

#include "Avr8GenericCommandFrame.hpp"
#include "../../ResponseFrames/AVR8Generic/GetDeviceId.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::CommandFrames::Avr8Generic
{
    class GetDeviceId: public Avr8GenericCommandFrame<std::array<unsigned char, 2>>
    {
    public:
        using ExpectedResponseFrameType = ResponseFrames::Avr8Generic::GetDeviceId;

        GetDeviceId() {
            /*
             * The get device ID command consists of 2 bytes:
             * 1. Command ID (0x12)
             * 2. Version (0x00)
             */
            this->payload = {
                0x12,
                0x00
            };
        }
    };
}
