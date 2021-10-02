#include "Response.hpp"

#include "src/Exceptions/Exception.hpp"

using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap;

void Response::init(const std::vector<unsigned char>& rawResponse) {
    if (rawResponse.empty()) {
        throw Exceptions::Exception("Failed to process CMSIS-DAP response - invalid response");
    }

    this->setResponseId(rawResponse[0]);
    this->setData(std::vector<unsigned char>(rawResponse.begin() + 1, rawResponse.end()));
}
