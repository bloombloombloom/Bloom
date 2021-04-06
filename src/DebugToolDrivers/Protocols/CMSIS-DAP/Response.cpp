#include "src/Exceptions/Exception.hpp"
#include "Response.hpp"

using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap;

void Response::init(const std::vector<unsigned char>& rawResponse)
{
    if (rawResponse.size() < 1) {
        throw Exceptions::Exception("Failed to process CMSIS-DAP response - invalid response");
    }

    this->setResponseId(rawResponse[0]);
    this->setData(std::vector<unsigned char>(rawResponse.begin() + 1, rawResponse.end()));
}

