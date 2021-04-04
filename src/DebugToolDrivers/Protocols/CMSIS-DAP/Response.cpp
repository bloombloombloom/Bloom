#include <cstdint>

#include "src/Exceptions/Exception.hpp"
#include "Response.hpp"

using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap;

void Response::init(unsigned char* response, std::size_t length)
{
    if (length == 0) {
        throw Exceptions::Exception("Failed to process CMSIS-DAP response - invalid response");
    }

    this->setResponseId(response[0]);
    std::vector<unsigned char> data = this->getData();

    // TODO: use insert with iterators here
    for (std::size_t i = 1; i < length; i++) {
        data.push_back(response[i]);
    }

    this->setData(data);
}

