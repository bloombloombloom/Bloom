#include "HouseKeepingResponseFrame.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::HouseKeeping
{
    using namespace Bloom::Exceptions;

    ResponseId HouseKeepingResponseFrame::getResponseId() {
        const auto& payload = this->getPayload();

        if (payload.empty()) {
            throw Exception("Response ID missing from HOUSEKEEPING response frame payload.");
        }

        return static_cast<ResponseId>(payload[0]);
    }
}
