#include "AvrEvent.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    using namespace Exceptions;

    AvrEvent::AvrEvent(const std::vector<unsigned char>& rawResponse)
        : Response(rawResponse)
    {
        if (this->id != 0x82) {
            throw Exception("Failed to construct AvrEvent object - invalid response ID.");
        }

        if (this->data.size() < 7) {
            throw Exception("Failed to construct AvrEvent object - unexpected size of AVR_EVT response.");
        }

        // Response size is two bytes, MSB
        const auto responsePacketSize = static_cast<std::size_t>((this->data[0] << 8) | this->data[1]);

        if (responsePacketSize == 0) {
            // No event available
            return;
        }

        if (this->data.size() < responsePacketSize + 7) {
            throw Exception("Failed to construct AvrEvent object - invalid size of AVR_EVT response packet.");
        }

        /*
         * Ignore the SOF, protocol version, handler ID, sequence ID and size bytes (which all make up 7 bytes
         * in total).
         */
        this->eventData = std::vector<unsigned char>(
            this->data.begin() + 7,
            this->data.begin() + 7 + static_cast<std::int64_t>(responsePacketSize)
        );

        this->eventId = static_cast<AvrEventId>(this->eventData[0]);
    }
}
