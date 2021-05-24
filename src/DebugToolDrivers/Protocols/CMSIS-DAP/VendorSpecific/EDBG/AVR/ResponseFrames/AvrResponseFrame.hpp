#pragma once

#include <cstdint>
#include <vector>
#include <algorithm>
#include <memory>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/Edbg.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrResponse.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    class AvrResponseFrame
    {
    private:
        unsigned char SOF = 0x0E;

        /**
         * Incrementing from 0x00
         */
        std::uint16_t sequenceID = 1;

        /**
         * Destination sub-protocol handler ID
         */
        ProtocolHandlerId protocolHandlerID;

        std::vector<unsigned char> payload;

    protected:
        virtual void initFromRawFrame(const std::vector<unsigned char>& rawFrame);

        void setSequenceId(std::uint16_t sequenceId) {
            this->sequenceID = sequenceId;
        }

        void setProtocolHandlerId(ProtocolHandlerId protocolHandlerId) {
            this->protocolHandlerID = protocolHandlerId;
        }

        void setProtocolHandlerId(unsigned char protocolHandlerId) {
            this->protocolHandlerID = static_cast<ProtocolHandlerId>(protocolHandlerId);
        }

        void setPayload(const std::vector<unsigned char>& payload) {
            this->payload = payload;
        }

    public:
        explicit AvrResponseFrame(const std::vector<AvrResponse>& AVRResponses) {
            this->initFromAvrResponses(AVRResponses);
        }

        explicit AvrResponseFrame() {}

        /**
         * An AVRResponse contains a single fragment of an AvrResponseFrame.
         *
         * This method will construct an AvrResponseFrame from a vector of AVRResponses.
         *
         * @param avrResponses
         */
        void initFromAvrResponses(const std::vector<AvrResponse>& avrResponses);

        std::uint16_t getSequenceId() const {
            return this->sequenceID;
        }

        ProtocolHandlerId getProtocolHandlerId() const {
            return this->protocolHandlerID;
        }

        std::vector<unsigned char>& getPayload() {
            return this->payload;
        }

        unsigned char getResponseId() {
            return this->payload[0];
        }

        virtual std::vector<unsigned char> getPayloadData() {
            return this->getPayload();
        }
    };
}
