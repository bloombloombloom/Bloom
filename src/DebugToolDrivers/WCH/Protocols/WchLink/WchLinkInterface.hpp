#pragma once

#include <memory>
#include <chrono>
#include <optional>
#include <vector>
#include <span>
#include <utility>

#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/DebugTransportModuleInterface.hpp"
#include "src/DebugToolDrivers/TargetInterfaces/RiscV/RiscVProgramInterface.hpp"
#include "src/DebugToolDrivers/TargetInterfaces/RiscV/RiscVIdentificationInterface.hpp"

#include "src/DebugToolDrivers/USB/UsbInterface.hpp"
#include "src/DebugToolDrivers/USB/UsbDevice.hpp"

#include "src/DebugToolDrivers/WCH/WchGeneric.hpp"
#include "Commands/Command.hpp"

#include "src/DebugToolDrivers/WCH/DeviceInfo.hpp"

#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink
{
    /**
     * Implementation of the WCH-Link protocol, which provides an implementation of a RISC-V DTM interface, and a
     * target identification interface.
     */
    class WchLinkInterface
        : public ::DebugToolDrivers::Protocols::RiscVDebugSpec::DebugTransportModuleInterface
        , public TargetInterfaces::RiscV::RiscVIdentificationInterface
    {
    public:
        WchLinkInterface(Usb::UsbInterface& usbInterface, Usb::UsbDevice& usbDevice);

        DeviceInfo getDeviceInfo();
        void activate() override;
        void deactivate() override;
        std::string getDeviceId() override;

        ::DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::RegisterValue readDebugModuleRegister(
            ::DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::RegisterAddress address
        ) override;
        void writeDebugModuleRegister(
            ::DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::RegisterAddress address,
            ::DebugToolDrivers::Protocols::RiscVDebugSpec::DebugModule::RegisterValue value
        ) override;

        void writePartialPage(Targets::TargetMemoryAddress startAddress, Targets::TargetMemoryBufferSpan buffer);

        void writeFullPage(
            Targets::TargetMemoryAddress startAddress,
            Targets::TargetMemoryBufferSpan buffer,
            Targets::TargetMemorySize pageSize,
            std::span<const unsigned char> flashProgramOpcodes
        );

        void eraseChip();

    private:
        static constexpr std::uint8_t USB_COMMAND_ENDPOINT_IN = 0x81;
        static constexpr std::uint8_t USB_COMMAND_ENDPOINT_OUT = 0x01;
        static constexpr std::uint8_t USB_DATA_ENDPOINT_IN = 0x82;
        static constexpr std::uint8_t USB_DATA_ENDPOINT_OUT = 0x02;
        static constexpr std::uint8_t DMI_OP_MAX_RETRY = 10;

        Usb::UsbInterface& usbInterface;

        std::uint16_t commandEndpointMaxPacketSize = 0;
        std::uint16_t dataEndpointMaxPacketSize = 0;
        // TODO: Move this into a config param
        std::chrono::microseconds dmiOpRetryDelay = std::chrono::microseconds{10};

        /**
         * The 'target activation' command returns a payload of 5 bytes.
         *
         * The last 4 bytes hold the WCH target variant ID. Given that the 'target activation' command appears to be
         * the only way to obtain this ID, we cache it via WchLinkInterface::cachedVariantId and return the cached
         * value in WchLinkInterface::getTargetId().
         */
        std::optional<WchTargetVariantId> cachedVariantId;
        std::optional<WchTargetId> cachedTargetId;

        void setClockSpeed(WchLinkTargetClockSpeed speed);

        template <class CommandType>
        auto sendCommandAndWaitForResponse(const CommandType& command) {
            const auto rawCommand = command.getRawCommand();

            /*
             * Although the UsbInterface::writeBulk() will split the transfer, if it exceeds the max packet size, we
             * still expect all commands to not exceed that size.
             */
            if (rawCommand.size() > this->commandEndpointMaxPacketSize) {
                throw Exceptions::DeviceCommunicationFailure{
                    "Raw command size exceeds maximum packet size for command endpoint"
                };
            }

            this->usbInterface.writeBulk(
                WchLinkInterface::USB_COMMAND_ENDPOINT_OUT,
                rawCommand,
                this->commandEndpointMaxPacketSize
            );

            const auto rawResponse = this->usbInterface.readBulk(WchLinkInterface::USB_COMMAND_ENDPOINT_IN);

            if (rawResponse.size() < 4) {
                throw Exceptions::DeviceCommunicationFailure{"Invalid response size from device"};
            }

            // The first byte of the response should be 0x82 (for success) or 0x81 (for failure)
            if (rawResponse[0] != 0x81 && rawResponse[0] != 0x82) {
                throw Exceptions::DeviceCommunicationFailure{"Invalid response code from device"};
            }

            if (rawResponse[0] == 0x81) {
                // TODO: Create ErrorResponse exception class and throw an instance of it here.
                throw Exceptions::DeviceCommunicationFailure{"Error response"};
            }

            if (rawResponse[1] != command.commandId) {
                throw Exceptions::DeviceCommunicationFailure{"Missing/invalid command ID in response from device"};
            }

            if ((rawResponse.size() - 3) != rawResponse[2]) {
                throw Exceptions::DeviceCommunicationFailure{"Actual response payload size mismatch"};
            }

            return typename CommandType::ExpectedResponseType{
                std::vector<unsigned char>{rawResponse.begin() + 3, rawResponse.end()}
            };
        }
    };
}
