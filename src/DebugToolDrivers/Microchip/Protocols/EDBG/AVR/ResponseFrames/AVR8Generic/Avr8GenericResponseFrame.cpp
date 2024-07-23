#include "Avr8GenericResponseFrame.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::ResponseFrames::Avr8Generic
{
    using namespace Exceptions;

    Avr8GenericResponseFrame::Avr8GenericResponseFrame(const std::vector<AvrResponse>& avrResponses)
        : AvrResponseFrame(avrResponses)
    {
        if (this->payload.empty()) {
            throw Exception{"Response ID missing from AVR8 Generic response frame payload."};
        }

        this->id = static_cast<Avr8ResponseId>(this->payload[0]);
    }

    std::vector<unsigned char> Avr8GenericResponseFrame::getPayloadData() const {
        /*
         * AVR8 data payloads are in little endian form and include two bytes before the data (response ID and
         * version byte) as well as an additional byte after the data, known as the 'status code'.
         */
        auto data = std::vector<unsigned char>{this->payload.begin() + 2, this->payload.end() - 1};
        std::reverse(data.begin(), data.end());
        return data;
    }
}
