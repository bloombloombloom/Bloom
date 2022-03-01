#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Avr8Generic
{
    class Avr8GenericResponseFrame: public AvrResponseFrame
    {
    public:
        Avr8GenericResponseFrame() = default;
        explicit Avr8GenericResponseFrame(const std::vector<AvrResponse>& avrResponses)
        : AvrResponseFrame(avrResponses) {}

        [[nodiscard]] unsigned char getResponseId();

        /**
         * See parent method.
         */
        [[nodiscard]] std::vector<unsigned char> getPayloadData() override;
    };
}
