#include "AvrIspResponseFrame.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::AvrIsp
{
    using namespace Bloom::Exceptions;

    StatusCode AvrIspResponseFrame::getStatusCode() {
        const auto& payload = this->getPayload();

        if (payload.size() < 2) {
            throw Exception("Status code missing from AVRISP response frame payload.");
        }

        return static_cast<StatusCode>(payload[1]);
    }
}
