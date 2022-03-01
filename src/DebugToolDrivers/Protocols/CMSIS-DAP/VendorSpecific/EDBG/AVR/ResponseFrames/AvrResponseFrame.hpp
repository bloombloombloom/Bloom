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
    public:
        explicit AvrResponseFrame() = default;

        explicit AvrResponseFrame(const std::vector<AvrResponse>& AVRResponses) {
            this->initFromAvrResponses(AVRResponses);
        }

        virtual ~AvrResponseFrame() = default;

        AvrResponseFrame(const AvrResponseFrame& other) = default;
        AvrResponseFrame(AvrResponseFrame&& other) = default;

        AvrResponseFrame& operator = (const AvrResponseFrame& other) = default;
        AvrResponseFrame& operator = (AvrResponseFrame&& other) = default;

        /**
         * An AvrResponse contains a single fragment of an AvrResponseFrame.
         *
         * This method will construct an AvrResponseFrame from a vector of AVRResponses.
         *
         * @param avrResponses
         */
        void initFromAvrResponses(const std::vector<AvrResponse>& avrResponses);

        [[nodiscard]] std::uint16_t getSequenceId() const {
            return this->sequenceID;
        }

        [[nodiscard]] ProtocolHandlerId getProtocolHandlerId() const {
            return this->protocolHandlerID;
        }

        std::vector<unsigned char>& getPayload() {
            return this->payload;
        }

        [[nodiscard]] virtual std::vector<unsigned char> getPayloadData() {
            return this->payload;
        }

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

    private:
        unsigned char SOF = 0x0E;

        /**
         * Incrementing from 0x00
         */
        std::uint16_t sequenceID = 1;

        /**
         * Destination sub-protocol handler ID
         */
        ProtocolHandlerId protocolHandlerID = ProtocolHandlerId::AVR8_GENERIC;

        std::vector<unsigned char> payload;
    };
}
