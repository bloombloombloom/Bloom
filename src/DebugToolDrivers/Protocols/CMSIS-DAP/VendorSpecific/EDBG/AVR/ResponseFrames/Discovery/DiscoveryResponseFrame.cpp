#include "DiscoveryResponseFrame.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::Discovery
{
    using namespace Bloom::Exceptions;

    DiscoveryResponseFrame::DiscoveryResponseFrame(const std::vector<AvrResponse>& avrResponses)
        : AvrResponseFrame(avrResponses)
    {
        if (this->payload.empty()) {
            throw Exception("Response ID missing from DISCOVERY response frame payload.");
        }

        this->id = static_cast<ResponseId>(payload[0]);
    }

    std::vector<unsigned char> DiscoveryResponseFrame::getPayloadData() const {
        if (this->payload.size() <= 2) {
            return {};
        }

        // DISCOVERY payloads include two bytes before the data (response ID and version byte).
        return std::vector<unsigned char>(
            this->payload.begin() + 2,
            this->payload.end()
        );
    }
}
