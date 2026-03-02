#include "HouseKeepingResponseFrame.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr::ResponseFrames::HouseKeeping
{
    using namespace Exceptions;

    HouseKeepingResponseFrame::HouseKeepingResponseFrame(const std::vector<AvrResponse>& avrResponses)
        : AvrResponseFrame(avrResponses)
    {
        if (this->payload.size() < 2) {
            throw Exception{"Response ID missing from HOUSEKEEPING response frame payload."};
        }

        this->id = static_cast<ResponseId>(this->payload[0]);
        this->version = this->payload[1];
    }

    std::vector<unsigned char> HouseKeepingResponseFrame::getPayloadData() const {
        if (this->payload.size() <= 2) {
            return {};
        }

        /*
         * The EDBG protocol document describes a trailing byte in all DATA responses, to indicate the status of a
         * read operation. I don't think this is something we should be checking for in here. The higher level code
         * should do it. TODO: Review
         */

        return {this->payload.begin() + 2, this->payload.end()};
    }
}
