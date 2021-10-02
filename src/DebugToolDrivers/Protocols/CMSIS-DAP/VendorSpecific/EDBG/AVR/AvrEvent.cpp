#include "AvrEvent.hpp"

#include "src/Exceptions/Exception.hpp"

using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr;
using namespace Bloom::Exceptions;

void AvrEvent::init(const std::vector<unsigned char>& rawResponse) {
    Response::init(rawResponse);

    if (this->getResponseId() != 0x82) {
        throw Exception("Failed to construct AvrEvent object - invalid response ID.");
    }

    auto& responseData = this->getData();

    if (responseData.size() < 2) {
        // All AVR_EVT responses should consist of at least two bytes (excluding the AVR_EVT ID)
        throw Exception("Failed to construct AvrEvent object - AVR_EVT response "
                        "returned no additional data.");
    }

    // Response size is two bytes, MSB
    auto responsePacketSize = static_cast<size_t>((responseData[0] << 8) | responseData[1]);

    if (responseData.size() < 2) {
        // All AVR_EVT responses should consist of at least two bytes (excluding the AVR_EVT ID)
        throw Exception("Failed to construct AvrEvent object - AVR_EVT response "
                        "contained invalid event data size.");
    }

    auto eventData = std::vector<unsigned char>();

    /*
     * Ignore the SOF, protocol version  &handler id and sequence ID (with all make up 5 bytes in total, 7 when
     * you include the two size bytes)
     */
    eventData.insert(
        eventData.end(),
        responseData.begin() + 7,
        responseData.begin() + 7 + static_cast<long>(responsePacketSize)
    );

    this->setEventData(eventData);

    if (!eventData.empty()) {
        this->eventId = eventData[0];
    }
}
