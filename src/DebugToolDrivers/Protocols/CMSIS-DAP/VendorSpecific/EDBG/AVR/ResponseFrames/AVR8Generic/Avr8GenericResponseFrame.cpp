#include "Avr8GenericResponseFrame.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Avr8Generic
{
    using namespace Bloom::Exceptions;

    unsigned char Avr8GenericResponseFrame::getResponseId() {
        const auto& payload = this->getPayload();
        if (payload.empty()) {
            throw Exception("Response ID missing from AVR8 Generic response frame payload.");
        }

        return payload[0];
    }

    std::vector<unsigned char> Avr8GenericResponseFrame::getPayloadData() {
        const auto& payload = this->getPayload();

        /*
         * AVR8 data payloads are in little endian form and include two bytes before the data (response ID and
         * version byte) as well as an additional byte after the data, known as the 'status code'.
         */
        auto data = std::vector<unsigned char>(
            payload.begin() + 2,
            payload.end() - 1
        );

        std::reverse(data.begin(), data.end());
        return data;
    }
}
