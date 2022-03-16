#include "EdbgControlResponseFrame.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr::ResponseFrames::EdbgControl
{
    using namespace Bloom::Exceptions;

    EdbgControlResponseId EdbgControlResponseFrame::getResponseId() {
        const auto& payload = this->getPayload();
        if (payload.empty()) {
            throw Exception("Response ID missing from EDBG Control response frame payload.");
        }

        return static_cast<EdbgControlResponseId>(payload[0]);
    }

    std::vector<unsigned char> EdbgControlResponseFrame::getPayloadData() {
        const auto& payload = this->getPayload();

        /*
         * EDBG Control data payloads include two bytes before the data (response ID and version byte) as well as an
         * additional byte after the data, known as the 'status code'.
         */
        auto data = std::vector<unsigned char>(
            payload.begin() + 2,
            payload.end() - 1
        );

        return data;
    }
}
