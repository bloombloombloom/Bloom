#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "src/DebugToolDrivers/USB/UsbInterface.hpp"

#include "Commands/Command.hpp"

#include "src/DebugToolDrivers/WCH/DeviceInfo.hpp"

#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink
{
    class WchLinkInterface
    {
    public:
        explicit WchLinkInterface(Usb::UsbInterface& usbInterface);

        DeviceInfo getDeviceInfo();

    private:
        static constexpr std::uint8_t USB_ENDPOINT_IN = 0x81;
        static constexpr std::uint8_t USB_ENDPOINT_OUT = 0x01;

        Usb::UsbInterface& usbInterface;

        template <class CommandType>
        auto sendCommandAndWaitForResponse(const CommandType& command) {
            this->usbInterface.writeBulk(WchLinkInterface::USB_ENDPOINT_OUT, command.getRawCommand());

            const auto rawResponse = this->usbInterface.readBulk(WchLinkInterface::USB_ENDPOINT_IN);

            if (rawResponse.size() < 3) {
                throw Exceptions::DeviceCommunicationFailure("Invalid response size from device");
            }

            // The first byte of the response should be 0x82 (for success) or 0x81 (for failure)
            if ((rawResponse[0] != 0x81 && rawResponse[0] != 0x82)) {
                throw Exceptions::DeviceCommunicationFailure("Invalid response code from device");
            }

            if (rawResponse[0] == 0x81) {
                // TODO: Create ErrorResponse exception class and throw an instance of it here.
                throw Exceptions::DeviceCommunicationFailure("Error response");
            }

            if (rawResponse[1] != command.commandId) {
                throw Exceptions::DeviceCommunicationFailure("Missing/invalid command ID in response from device");
            }

            if ((rawResponse.size() - 3) != rawResponse[2]) {
                throw Exceptions::DeviceCommunicationFailure("Actual response payload size mismatch");
            }

            return typename CommandType::ExpectedResponseType(
                std::vector<unsigned char>(rawResponse.begin() + 3, rawResponse.end())
            );
        }
    };
}
