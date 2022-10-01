#include "AvrIspResponseFrame.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::AvrIsp
{
    using namespace Bloom::Exceptions;

    AvrIspResponseFrame::AvrIspResponseFrame(const std::vector<AvrResponse>& avrResponses)
        : AvrResponseFrame(avrResponses)
    {
        if (this->payload.size() < 2) {
            throw Exception("Status code missing from AVRISP response frame payload.");
        }

        this->statusCode = static_cast<StatusCode>(this->payload[1]);
    }
}
