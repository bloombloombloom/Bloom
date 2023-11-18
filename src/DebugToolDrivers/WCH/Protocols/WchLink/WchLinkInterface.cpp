#include "WchLinkInterface.hpp"

#include <memory>

#include "src/Helpers/BiMap.hpp"

#include "Commands/Control/GetDeviceInfo.hpp"

#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink
{
    using namespace Exceptions;

    WchLinkInterface::WchLinkInterface(Usb::UsbInterface& usbInterface)
        : usbInterface(usbInterface)
    {}

    DeviceInfo WchLinkInterface::getDeviceInfo() {
        const auto response = this->sendCommandAndWaitForResponse(
            Commands::Control::GetDeviceInfo()
        );

        if (response.payload.size() < 3) {
            throw Exceptions::DeviceCommunicationFailure("Cannot construct DeviceInfo response - invalid payload");
        }

        static const auto variantsById = BiMap<std::uint8_t, WchLinkVariant>({
            {0x01, WchLinkVariant::LINK_CH549},
            {0x02, WchLinkVariant::LINK_E_CH32V307},
            {0x03, WchLinkVariant::LINK_S_CH32V203},
        });

        return DeviceInfo(
            WchFirmwareVersion(response.payload[0], response.payload[1]),
            response.payload.size() >= 4
                ? std::optional(variantsById.valueAt(response.payload[2]).value_or(WchLinkVariant::UNKNOWN))
                : std::nullopt
        );
    }
}
