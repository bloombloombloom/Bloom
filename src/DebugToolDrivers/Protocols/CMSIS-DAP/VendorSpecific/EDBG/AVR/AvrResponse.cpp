#include "AvrResponse.hpp"

#include "src/Exceptions/Exception.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    using namespace Bloom::Exceptions;

    void AvrResponse::init(const std::vector<unsigned char>& rawResponse) {
        Response::init(rawResponse);

        if (this->getResponseId() != 0x81) {
            throw Exception("Failed to construct AvrResponse object - invalid response ID.");
        }

        const auto& responseData = this->getData();

        if (responseData.empty()) {
            // All AVR responses should contain at least one byte (the fragment info byte)
            throw Exception("Failed to construct AvrResponse object - AVR_RSP response "
                "returned no additional data");
        }

        if (responseData[0] == 0x00) {
            // This AVR Response contains no data (the device had no data to send), so we can stop here.
            return;
        }

        this->setFragmentCount(static_cast<std::uint8_t>(responseData[0] & 0x0FU));
        this->setFragmentNumber(static_cast<std::uint8_t>(responseData[0] >> 4));

        // Response size is two bytes, MSB
        const auto responsePacketSize = static_cast<std::size_t>((responseData[1] << 8U) + responseData[2]);
        std::vector<unsigned char> responsePacket;
        responsePacket.resize(responsePacketSize);

        for (std::size_t i = 0; i < responsePacketSize; i++) {
            responsePacket[i] = responseData[i + 3];
        }

        this->setResponsePacket(responsePacket);
    }
}
