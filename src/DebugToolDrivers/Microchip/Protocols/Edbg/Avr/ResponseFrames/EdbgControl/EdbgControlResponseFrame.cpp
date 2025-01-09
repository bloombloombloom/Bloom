#include "EdbgControlResponseFrame.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::ResponseFrames::EdbgControl
{
    using namespace Exceptions;

    EdbgControlResponseFrame::EdbgControlResponseFrame(const std::vector<AvrResponse>& avrResponses)
        : AvrResponseFrame(avrResponses)
    {
        if (this->payload.empty()) {
            throw Exception{"Response ID missing from EDBG Control response frame payload."};
        }

        this->id = static_cast<EdbgControlResponseId>(this->payload[0]);
    }

    std::vector<unsigned char> EdbgControlResponseFrame::getPayloadData() {
        if (this->payload.size() <= 3) {
            return {};
        }

        /*
         * EDBG Control data payloads include two bytes before the data (response ID and version byte) as well as an
         * additional byte after the data, known as the 'status code'.
         */
        return {this->payload.begin() + 2, this->payload.end() - 1};
    }
}