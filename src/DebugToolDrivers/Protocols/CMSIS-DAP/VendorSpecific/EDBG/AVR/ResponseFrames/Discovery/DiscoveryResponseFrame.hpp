#pragma once

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames
{
    class DiscoveryResponseFrame: public AvrResponseFrame
    {
    public:
        DiscoveryResponseFrame() = default;
        explicit DiscoveryResponseFrame(const std::vector<AvrResponse>& AVRResponses): AvrResponseFrame(AVRResponses) {}

        unsigned char getResponseId();

        /**
         * See parent method.
         */
        std::vector<unsigned char> getPayloadData() override;
    };
}
