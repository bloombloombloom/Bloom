#include "AvrResponseFrame.hpp"

#include "src/Exceptions/Exception.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    using namespace Exceptions;

    void AvrResponseFrame::initFromAvrResponses(const std::vector<AvrResponse>& avrResponses) {
        // Build a raw frame buffer from the AvrResponse objects and just call initFromRawFrame()
        auto rawFrame = std::vector<unsigned char>{};

        for (const auto& avrResponse : avrResponses) {
            rawFrame.insert(rawFrame.end(), avrResponse.responsePacket.begin(), avrResponse.responsePacket.end());
        }

        return this->initFromRawFrame(rawFrame);
    }

    void AvrResponseFrame::initFromRawFrame(const std::vector<unsigned char>& rawFrame) {
        if (rawFrame.size() < 4) {
            /*
             * All AVR response frames must consist of at least four bytes (SOF, sequence ID (two bytes) and
             * a protocol handler ID).
             */
            throw Exception{"Failed to construct AvrResponseFrame - unexpected end to raw frame"};
        }

        if (rawFrame[0] != 0x0E) {
            // Invalid SOF byte value
            throw Exception{"Failed to construct AvrResponseFrame - unexpected SOF byte value in raw frame"};
        }

        this->sequenceId = static_cast<std::uint16_t>((rawFrame[2] << 8) + rawFrame[1]);
        this->protocolHandlerId = static_cast<ProtocolHandlerId>(rawFrame[3]);

        this->payload.insert(payload.begin(), rawFrame.begin() + 4, rawFrame.end());
    }
}
