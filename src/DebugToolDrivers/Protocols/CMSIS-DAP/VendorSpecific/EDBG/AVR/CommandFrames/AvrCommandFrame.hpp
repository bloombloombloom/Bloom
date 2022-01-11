#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <limits>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/Edbg.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrCommand.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    class AvrCommandFrame
    {
    public:
        using ResponseFrameType = AvrResponseFrame;

        AvrCommandFrame() {
            if (AvrCommandFrame::lastSequenceId < std::numeric_limits<decltype(AvrCommandFrame::lastSequenceId)>::max()) {
                this->sequenceId = ++(AvrCommandFrame::lastSequenceId);

            } else {
                this->sequenceId = 0;
                AvrCommandFrame::lastSequenceId = 0;
            }
        };
        virtual ~AvrCommandFrame() = default;

        AvrCommandFrame(const AvrCommandFrame& other) = default;
        AvrCommandFrame(AvrCommandFrame&& other) = default;

        AvrCommandFrame& operator = (const AvrCommandFrame& other) = default;
        AvrCommandFrame& operator = (AvrCommandFrame&& other) = default;

        [[nodiscard]] unsigned char getProtocolVersion() const {
            return this->protocolVersion;
        }

        void setProtocolVersion(unsigned char protocolVersion) {
            this->protocolVersion = protocolVersion;
        }

        [[nodiscard]] std::uint16_t getSequenceId() const {
            return this->sequenceId;
        }

        [[nodiscard]] ProtocolHandlerId getProtocolHandlerId() const {
            return this->protocolHandlerID;
        }

        void setProtocolHandlerId(ProtocolHandlerId protocolHandlerId) {
            this->protocolHandlerID = protocolHandlerId;
        }

        void setProtocolHandlerId(unsigned char protocolHandlerId) {
            this->protocolHandlerID = static_cast<ProtocolHandlerId>(protocolHandlerId);
        }

        [[nodiscard]] virtual std::vector<unsigned char> getPayload() const {
            return this->payload;
        }

        void setPayload(const std::vector<unsigned char>& payload) {
            this->payload = payload;
        }

        /**
         * AvrCommandFrames are sent to the device via AVRCommands (CMSIS-DAP vendor commands).
         *
         * If the size of an AvrCommandFrame exceeds the maximum packet size of an AVRCommand, it will need to
         * be split into multiple AVRCommands before being sent to the device.
         *
         * This methods generates AVR commands from an AvrCommandFrame. The number of AVRCommands generated depends
         * on the size of the AvrCommandFrame and the passed maximumCommandPacketSize.
         *
         * @param maximumCommandPacketSize
         *  The maximum size of an AVRCommand command packet. This is usually the REPORT_SIZE of the HID
         *  endpoint, minus a few bytes to account for other AVRCommand fields. The maximumCommandPacketSize is used to
         *  determine the number of AVRCommands to be generated.
         *
         * @return
         *  A vector of sequenced AVRCommands, each containing a segment of the AvrCommandFrame.
         */
        [[nodiscard]] std::vector<AvrCommand> generateAvrCommands(std::size_t maximumCommandPacketSize) const;

        /**
         * Converts instance of a CMSIS Command to an unsigned char, for sending to the Atmel ICE device.
         *
         * @return
         */
        explicit virtual operator std::vector<unsigned char> () const;

    private:
        unsigned char SOF = 0x0E;

        unsigned char protocolVersion = 0x00;

        /**
         * Incrementing from 0x00
         */
        std::uint16_t sequenceId = 0;
        inline static std::uint16_t lastSequenceId = 0;

        /**
         * Destination sub-protocol handler ID
         */
        ProtocolHandlerId protocolHandlerID = ProtocolHandlerId::DISCOVERY;

        std::vector<unsigned char> payload;
    };

}
