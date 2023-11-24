#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <memory>

#include "src/DebugToolDrivers/Microchip/Protocols/EDBG/Edbg.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/EDBG/AVR/AvrResponse.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    class AvrResponseFrame
    {
    public:
        /**
         * Incrementing from 0x00
         */
        std::uint16_t sequenceId = 0;

        /**
         * Destination sub-protocol handler ID
         */
        ProtocolHandlerId protocolHandlerId = ProtocolHandlerId::AVR8_GENERIC;

        std::vector<unsigned char> payload;

        explicit AvrResponseFrame(const std::vector<AvrResponse>& avrResponses) {
            this->initFromAvrResponses(avrResponses);
        }

        virtual ~AvrResponseFrame() = default;

        AvrResponseFrame(const AvrResponseFrame& other) = default;
        AvrResponseFrame(AvrResponseFrame&& other) = default;

        AvrResponseFrame& operator = (const AvrResponseFrame& other) = default;
        AvrResponseFrame& operator = (AvrResponseFrame&& other) = default;

        /**
         * An AvrResponse contains a single fragment of an AvrResponseFrame.
         *
         * This method will construct an AvrResponseFrame from a vector of AvrResponse.
         *
         * @param avrResponses
         */
        void initFromAvrResponses(const std::vector<AvrResponse>& avrResponses);

    private:
        virtual void initFromRawFrame(const std::vector<unsigned char>& rawFrame);
    };
}