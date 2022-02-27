#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <limits>
#include <cmath>
#include <atomic>
#include <array>

#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/Command.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/Edbg.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/AvrCommand.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/ResponseFrames/AvrResponseFrame.hpp"

namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr
{
    static inline std::atomic<std::uint16_t> LAST_SEQUENCE_ID = 0;

    template <class PayloadContainerType = std::vector<unsigned char>>
    class AvrCommandFrame
    {
        /*
         * Every AVR command frame contains a payload. For many commands, the payload size is fixed, meaning we can
         * use automatic storage. In other cases, the size of the payload is determined at runtime, requiring the use
         * of dynamic storage.
         *
         * For example, consider the Get Device ID command from the AVR8 Generic Protocol. The size of the payload
         * for this command is fixed at 2 bytes. So there is no need to use dynamic storage duration for the payload.
         *
         * Now consider the Write Memory command from the same protocol. The payload size for that command depends
         * on the size of memory we wish to write, which is not known at compile time. For this command, the payload
         * would have dynamic storage.
         *
         * For the above reason, the AvrCommandFrame class is a template class in which the payload container type can
         * be specified for individual commands.
         *
         * For now, we only permit two payload container types:
         *  - std::array<unsigned char> for payloads with automatic storage duration.
         *  - std::vector<unsigned char> for payloads with dynamic storage duration.
         */
        static_assert(
            (
                std::is_same_v<PayloadContainerType, std::vector<unsigned char>>
                || std::is_same_v<typename PayloadContainerType::value_type, unsigned char>
            ),
            "Invalid payload container type - must be an std::array<unsigned char, X> or an std::vector<unsigned char>"
        );


    public:
        using ResponseFrameType = AvrResponseFrame;

        AvrCommandFrame() {
            if (LAST_SEQUENCE_ID < std::numeric_limits<decltype(LAST_SEQUENCE_ID)>::max()) {
                this->sequenceId = ++(LAST_SEQUENCE_ID);

            } else {
                this->sequenceId = 0;
                LAST_SEQUENCE_ID = 0;
            }
        };

        virtual ~AvrCommandFrame() = default;

        AvrCommandFrame(const AvrCommandFrame& other) = default;
        AvrCommandFrame(AvrCommandFrame&& other) noexcept = default;

        AvrCommandFrame& operator = (const AvrCommandFrame& other) = default;
        AvrCommandFrame& operator = (AvrCommandFrame&& other) noexcept = default;

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

        [[nodiscard]] virtual const PayloadContainerType& getPayload() const {
            return this->payload;
        };

        /**
         * Converts the command frame into a container of unsigned char - a raw buffer to send to the EDBG device.
         *
         * @return
         */
        [[nodiscard]] auto getRawCommandFrame() const {
            auto data = this->getPayload();
            const auto dataSize = data.size();

            auto rawCommand = std::vector<unsigned char>(5);

            rawCommand[0] = this->SOF;
            rawCommand[1] = this->getProtocolVersion();

            rawCommand[2] = static_cast<unsigned char>(this->getSequenceId());
            rawCommand[3] = static_cast<unsigned char>(this->getSequenceId() >> 8);

            rawCommand[4] = static_cast<unsigned char>(this->getProtocolHandlerId());

            if (dataSize > 0) {
                rawCommand.insert(
                    rawCommand.end(),
                    data.begin(),
                    data.end()
                );
            }

            return rawCommand;
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
        [[nodiscard]] std::vector<AvrCommand> generateAvrCommands(std::size_t maximumCommandPacketSize) const {
            auto rawCommandFrame = this->getRawCommandFrame();

            std::size_t commandFrameSize = rawCommandFrame.size();
            auto commandsRequired = static_cast<std::size_t>(
                std::ceil(static_cast<float>(commandFrameSize) / static_cast<float>(maximumCommandPacketSize))
            );

            std::vector<AvrCommand> avrCommands;
            std::size_t copiedPacketSize = 0;
            for (std::size_t i = 0; i < commandsRequired; i++) {
                // If we're on the last packet, the packet size will be what ever is left of the AvrCommandFrame
                std::size_t commandPacketSize = ((i + 1) != commandsRequired) ? maximumCommandPacketSize
                    : (commandFrameSize - (maximumCommandPacketSize * i));

                avrCommands.emplace_back(AvrCommand(
                    commandsRequired,
                    i + 1,
                    std::vector<unsigned char>(
                        rawCommandFrame.begin() + static_cast<std::int64_t>(copiedPacketSize),
                        rawCommandFrame.begin() + static_cast<std::int64_t>(copiedPacketSize + commandPacketSize)
                    )
                ));
                copiedPacketSize += commandPacketSize;
            }

            return avrCommands;
        }

    protected:
        PayloadContainerType payload;

    private:
        unsigned char SOF = 0x0E;

        unsigned char protocolVersion = 0x00;

        /**
         * Incrementing from 0x00
         */
        std::uint16_t sequenceId = 0;

        /**
         * Destination sub-protocol handler ID
         */
        ProtocolHandlerId protocolHandlerID = ProtocolHandlerId::DISCOVERY;
    };

}
