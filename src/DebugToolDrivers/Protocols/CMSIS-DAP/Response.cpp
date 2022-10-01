#include "Response.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap
{
    Response::Response(const std::vector<unsigned char>& rawResponse) {
        if (rawResponse.empty()) {
            throw Exceptions::Exception("Failed to process CMSIS-DAP response - invalid response");
        }

        this->id = rawResponse[0];
        this->data = std::vector<unsigned char>(rawResponse.begin() + 1, rawResponse.end());
    }
}
