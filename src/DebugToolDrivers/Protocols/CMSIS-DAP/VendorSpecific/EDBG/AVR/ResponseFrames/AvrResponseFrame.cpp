#include <cstdint>

#include "AvrResponseFrame.hpp"
#include "src/Exceptions/Exception.hpp"

using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr;
using namespace Bloom::Exceptions;

void AvrResponseFrame::initFromAvrResponses(const std::vector<AvrResponse>& avrResponses) {
    // Build a raw frame buffer from the AVRResponse objects and just call initFromRawFrame()
    std::vector<unsigned char> rawFrame;

    for (auto& avrResponse : avrResponses) {
        auto responsePacket = avrResponse.getResponsePacket();
        rawFrame.insert(rawFrame.end(), responsePacket.begin(), responsePacket.end());
    }

    return this->initFromRawFrame(rawFrame);
}

void AvrResponseFrame::initFromRawFrame(const std::vector<unsigned char>& rawFrame) {
    if (rawFrame.size() < 4) {
        // All AVR response frames must consist of at least four bytes (SOF, sequence ID (two bytes) and
        // a protocol handler ID)
        throw Exception("Failed to construct AvrResponseFrame - unexpected end to raw frame");
    }

    if (rawFrame[0] != 0x0E) {
        // The SOF field must always be 0x0E
        throw Exception("Failed to construct AvrResponseFrame - unexpected SOF field value in raw frame");
    }

    this->setSequenceId(static_cast<std::uint16_t>((rawFrame[2] << 8) + rawFrame[1]));
    this->setProtocolHandlerId(rawFrame[3]);

    auto& payload = this->getPayload();
    payload.insert(payload.begin(), rawFrame.begin() + 4, rawFrame.end());
}