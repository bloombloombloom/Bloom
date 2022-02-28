#include "Response.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap
{
    Response::Response(const std::vector<unsigned char>& rawResponse) {
        if (rawResponse.empty()) {
            throw Exceptions::Exception("Failed to process CMSIS-DAP response - invalid response");
        }

        this->setResponseId(rawResponse[0]);
        this->setData(std::vector<unsigned char>(rawResponse.begin() + 1, rawResponse.end()));
    }
}
