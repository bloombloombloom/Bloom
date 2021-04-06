#pragma once

#include "AvrResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames
{
    class DiscoveryResponseFrame: public AvrResponseFrame
    {
    public:
        DiscoveryResponseFrame(const std::vector<AvrResponse>& AVRResponses): AvrResponseFrame(AVRResponses) {}
        DiscoveryResponseFrame() {}

        /**
         * See parent method.
         */
        std::vector<unsigned char> getPayloadData() override {
            /*
             * DISCOVERY payloads include two bytes before the data (response ID and version byte).
             */
            auto data = std::vector<unsigned char>(
                this->getPayload().begin() + 2,
                this->getPayload().end()
            );

            return data;
        }
    };

}
