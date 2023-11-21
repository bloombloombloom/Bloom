#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "src/DebugToolDrivers/DebugTool.hpp"
#include "src/DebugToolDrivers/USB/UsbDevice.hpp"

#include "Protocols/WchLink/WchLinkInterface.hpp"

#include "WchGeneric.hpp"
#include "DeviceInfo.hpp"

namespace DebugToolDrivers::Wch
{
    class WchLinkBase: public DebugTool, public Usb::UsbDevice
    {
    public:
        WchLinkBase(
            WchLinkVariant variant,
            std::uint16_t vendorId,
            std::uint16_t productId,
            std::uint8_t wchLinkUsbInterfaceNumber
        );

        void init() override;

        void close() override;

        std::string getSerialNumber() override;

        std::string getFirmwareVersionString() override;

        DebugToolDrivers::TargetInterfaces::RiscV::RiscVDebugInterface* getRiscVDebugInterface() override {
            return this->wchLinkInterface.get();
        }

    protected:
        WchLinkVariant variant;

        std::uint8_t wchLinkUsbInterfaceNumber;
        std::unique_ptr<Usb::UsbInterface> wchLinkUsbInterface = nullptr;
        std::unique_ptr<Protocols::WchLink::WchLinkInterface> wchLinkInterface = nullptr;

        mutable std::optional<DeviceInfo> cachedDeviceInfo = std::nullopt;

        const DeviceInfo& getDeviceInfo() const;
    };
}
