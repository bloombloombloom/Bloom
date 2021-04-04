#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Avr8Generic
{
    class Avr8GenericResponseFrame: public AvrResponseFrame
    {
    public:
        Avr8GenericResponseFrame(const std::vector<AvrResponse>& AVRResponses) : AvrResponseFrame(AVRResponses) {}
        Avr8GenericResponseFrame() {}

        /**
         * See parent method.
         */
        std::vector<unsigned char> getPayloadData() override {
            /*
             * AVR8 data payloads are in little endian form and include two bytes before the data (response ID and
             * version byte) as well as an additional byte after the data, known as the 'status code'.
             */
            auto data = std::vector<unsigned char>(
                this->getPayload().begin() + 2,
                this->getPayload().end() - 1
            );

            std::reverse(data.begin(), data.end());
            return data;
        }
    };

}
