#include "AvrResponse.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    using namespace Exceptions;

    AvrResponse::AvrResponse(const std::vector<unsigned char>& rawResponse)
        : Response(rawResponse)
    {
        if (this->id != 0x81) {
            throw Exception("Failed to construct AvrResponse object - invalid response ID.");
        }

        if (this->data.empty()) {
            // All AVR responses should contain at least one byte (the fragment info byte)
            throw Exception("Failed to construct AvrResponse object - malformed AVR_RSP data");
        }

        if (this->data[0] == 0x00) {
            // This AVR Response contains no data (the device had no data to send), so we can stop here.
            return;
        }

        this->fragmentCount = static_cast<std::uint8_t>(this->data[0] & 0x0FU);
        this->fragmentNumber = static_cast<std::uint8_t>(this->data[0] >> 4);

        // Response size is two bytes, MSB
        const auto responsePacketSize = static_cast<std::uint16_t>((this->data[1] << 8U) + this->data[2]);

        if (responsePacketSize > 0) {
            // Packet data
            this->responsePacket.insert(
                this->responsePacket.begin(),
                this->data.begin() + 3,
                this->data.begin() + 3 + responsePacketSize
            );
        }
    }
}
