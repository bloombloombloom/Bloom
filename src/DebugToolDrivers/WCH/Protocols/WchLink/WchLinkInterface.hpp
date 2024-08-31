#pragma once

#include <memory>
#include <optional>
#include <vector>
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
     * Implementation of the WCH-Link protocol, which provides an implementation of a RISC-V DTM interface, a
     * programming interface and a target identification interface.
     *
     * WCH debug tools cannot write to program memory via the target's RISC-V debug module, so we cannot program the
     * target via the tool's RISC-V DTM interface. Instead, the WCH-Link protocol provides a dedicated command for
     * writing to program memory, which is why this class implements the RISC-V programming interface.
     * See WchLinkInterface::writeFlashMemory() for more.
     */
    class WchLinkInterface
        : public ::DebugToolDrivers::Protocols::RiscVDebugSpec::DebugTransportModuleInterface
        , public TargetInterfaces::RiscV::RiscVProgramInterface
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

        void writeFlashMemory(
            Targets::TargetMemoryAddress startAddress,
            const Targets::TargetMemoryBuffer& buffer
        ) override;

    private:
        static constexpr std::uint8_t USB_COMMAND_ENDPOINT_IN = 0x81;
        static constexpr std::uint8_t USB_COMMAND_ENDPOINT_OUT = 0x01;
        static constexpr std::uint8_t USB_DATA_ENDPOINT_IN = 0x82;
        static constexpr std::uint8_t USB_DATA_ENDPOINT_OUT = 0x02;

        Usb::UsbInterface& usbInterface;

        std::uint16_t commandEndpointMaxPacketSize = 0;
        std::uint16_t dataEndpointMaxPacketSize = 0;

        /**
         * The 'target activation' command returns a payload of 5 bytes.
         *
         * The last 4 bytes hold the WCH RISC-V target ID. Given that the 'target activation' command appears to be
         * the only way to obtain the target ID, we cache it via WchLinkInterface::cachedTargetId and return the
         * cached value in WchLinkInterface::getTargetId().
         *
         * As for the first byte in the payload, I'm not really sure what it is. It appears to be some kind of
         * identifier for groups of WCH RISC-V targets. It's unclear. All I know is that it has some significance, as
         * it's expected in the payload of some other commands, such as the command to set clock speed. For this
         * reason, we have to keep hold of it via WchLinkInterface::cachedTargetGroupId.
         */
        std::optional<WchTargetId> cachedTargetId;
        std::optional<std::uint8_t> cachedTargetGroupId;

        void setClockSpeed(WchLinkTargetClockSpeed speed);

        template <class CommandType>
        auto sendCommandAndWaitForResponse(const CommandType& command) {
            auto rawCommand = command.getRawCommand();

            if (rawCommand.size() > this->commandEndpointMaxPacketSize) {
                throw Exceptions::DeviceCommunicationFailure{
                    "Raw command size exceeds maximum packet size for command endpoint"
                };
            }

            this->usbInterface.writeBulk(WchLinkInterface::USB_COMMAND_ENDPOINT_OUT, std::move(rawCommand));

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
