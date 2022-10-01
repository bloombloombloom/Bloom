#include "HouseKeepingResponseFrame.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::HouseKeeping
{
    using namespace Bloom::Exceptions;

    HouseKeepingResponseFrame::HouseKeepingResponseFrame(const std::vector<AvrResponse>& avrResponses)
        : AvrResponseFrame(avrResponses)
    {
        if (this->payload.empty()) {
            throw Exception("Response ID missing from HOUSEKEEPING response frame payload.");
        }

        this->id = static_cast<ResponseId>(this->payload[0]);
    }
}
