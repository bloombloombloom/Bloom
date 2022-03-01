#include "HouseKeepingResponseFrame.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames
{
    using namespace Bloom::Exceptions;

    unsigned char HouseKeepingResponseFrame::getResponseId() {
        const auto& payload = this->getPayload();

        if (payload.empty()) {
            throw Exception("Response ID missing from HOUSEKEEPING response frame payload.");
        }

        return payload[0];
    }
}
