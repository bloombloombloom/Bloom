#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <limits>
#include <cmath>
#include <atomic>
#include <array>

#include "src/DebugToolDrivers/Protocols/CmsisDap/Command.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Edbg.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/AvrCommand.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/Edbg/Avr/ResponseFrames/AvrResponseFrame.hpp"

namespace DebugToolDrivers::Microchip::Protocols::Edbg::Avr
{
    static inline std::atomic<std::uint16_t> lastSequenceId = 0;

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
        /*
         * All AVR command frames result in one or more response frames from the EDBG device. The structure and
         * contents of the response frame depends on the command frame that was sent.
         *
         * The ExpectedResponseFrameType alias is used to map command frame types to response frame types.
         * This is used in some template functions, such as EdbgInterface::sendAvrCommandFrameAndWaitForResponseFrame().
         * That function will use the alias when constructing and returning a response frame object.
         *
         * For example, consider the GetDeviceId command - this command instructs the EDBG device to extract the
         * signature from the connected AVR target, and return it in a response frame. The GetDeviceId command frame
         * maps to the GetDeviceId response frame type (via the ExpectedResponseFrameType alias). When we send the
         * command, the correct response frame object is returned:
         *
         *   EdbgInterface edbgInterface;
         *   CommandFrames::Avr8Generic::GetDeviceId getDeviceIdCommandFrame;
         *
         *   auto responseFrame = edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(getDeviceIdCommandFrame);
         *   Targets::Microchip::Avr8::TargetSignature avrSignature = responseFrame->extractSignature();
         *
         * In the code above, the responseFrame object will be an instance of the ResponseFrames::Avr8Generic::GetDeviceId
         * class, which provides the extractSignature() function (to extract the AVR signature from the response frame).
         * This works because the EdbgInterface::sendAvrCommandFrameAndWaitForResponseFrame() function uses the
         * CommandFrames::Avr8Generic::GetDeviceId::ExpectedResponseFrameType alias to construct the response frame.
         *
         * For more, see the implementation of EdbgInterface::sendAvrCommandFrameAndWaitForResponseFrame().
         */
        using ExpectedResponseFrameType = AvrResponseFrame;

        /**
         * Incrementing from 0
         */
        std::uint16_t sequenceId = 0;

        /**
         * Destination sub-protocol handler ID
         */
        ProtocolHandlerId protocolHandlerId = ProtocolHandlerId::DISCOVERY;

        PayloadContainerType payload;

        explicit AvrCommandFrame(ProtocolHandlerId protocolHandlerId)
            : sequenceId(++lastSequenceId)
            , protocolHandlerId(protocolHandlerId)
        {};

        virtual ~AvrCommandFrame() = default;

        AvrCommandFrame(const AvrCommandFrame& other) = default;
        AvrCommandFrame(AvrCommandFrame&& other) noexcept = default;

        AvrCommandFrame& operator = (const AvrCommandFrame& other) = default;
        AvrCommandFrame& operator = (AvrCommandFrame&& other) noexcept = default;

        /**
         * Converts the command frame into a container of unsigned char - a raw buffer to include in an AVR
         * command packet, for sending to the EDBG device.
         *
         * See AvrCommandFrame::generateAvrCommands() and the AvrCommand class for more.
         *
         * @return
         */
        [[nodiscard]] auto getRawCommandFrame() const {
            auto rawCommand = std::vector<unsigned char>(5 + this->payload.size());

            rawCommand[0] = 0x0E; // Start of frame (SOF) byte
            rawCommand[1] = 0x00; // Protocol version

            rawCommand[2] = static_cast<unsigned char>(this->sequenceId);
            rawCommand[3] = static_cast<unsigned char>(this->sequenceId >> 8);

            rawCommand[4] = static_cast<unsigned char>(this->protocolHandlerId);

            if (!this->payload.empty()) {
                std::copy(this->payload.begin(), this->payload.end(), rawCommand.begin() + 5);
            }

            return rawCommand;
        }

        /**
         * AvrCommandFrames are sent to the device via AvrCommands (CMSIS-DAP vendor commands).
         *
         * If the size of an AvrCommandFrame exceeds the maximum packet size of an AVRCommand, it will need to
         * be split into multiple AvrCommands before being sent to the device.
         *
         * This methods generates AVR commands from an AvrCommandFrame. The number of AvrCommands generated depends
         * on the size of the AvrCommandFrame and the passed maximumCommandPacketSize.
         *
         * @param maximumCommandPacketSize
         *  The maximum size of an AVRCommand command packet. This is usually the REPORT_SIZE of the HID
         *  endpoint, minus a few bytes to account for other AVRCommand fields. The maximumCommandPacketSize is used to
         *  determine the number of AvrCommands to be generated.
         *
         * @return
         *  A vector of sequenced AvrCommands, each containing a segment of the AvrCommandFrame.
         */
        [[nodiscard]] std::vector<AvrCommand> generateAvrCommands(std::size_t maximumCommandPacketSize) const {
            const auto rawCommandFrame = this->getRawCommandFrame();

            const auto commandFrameSize = rawCommandFrame.size();
            const auto commandsRequired = static_cast<std::size_t>(
                std::ceil(static_cast<float>(commandFrameSize) / static_cast<float>(maximumCommandPacketSize))
            );

            auto avrCommands = std::vector<AvrCommand>{};
            auto copiedPacketSize = std::size_t{0};
            for (auto i = std::size_t{0}; i < commandsRequired; ++i) {
                // If we're on the last packet, the packet size will be what ever is left of the AvrCommandFrame
                const auto commandPacketSize = static_cast<std::size_t>(
                    ((i + 1) != commandsRequired)
                        ? maximumCommandPacketSize
                        : (commandFrameSize - (maximumCommandPacketSize * i))
                );

                avrCommands.emplace_back(AvrCommand(
                    commandsRequired,
                    i + 1,
                    std::vector<unsigned char>{
                        rawCommandFrame.begin() + static_cast<std::int64_t>(copiedPacketSize),
                        rawCommandFrame.begin() + static_cast<std::int64_t>(copiedPacketSize + commandPacketSize)
                    }
                ));
                copiedPacketSize += commandPacketSize;
            }

            return avrCommands;
        }
    };
}
