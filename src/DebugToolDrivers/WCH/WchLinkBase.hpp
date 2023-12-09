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

        /**
         * WCH-Link debug tools cannot write to flash memory via the RISC-V debug interface (RiscVDebugInterface).
         * Flash memory writes via abstract commands fail silently.
         *
         * We have to send a vendor-specific command to the debug tool, in order to program the target.
         *
         * For this reason, we have to provide an implementation of the RiscVProgramInterface, so that the RISC-V
         * target driver forwards any flash memory writes to this implementation (instead of relying on the debug
         * interface).
         *
         * The WchLinkInterface implements both the RiscVDebugInterface and the RiscVProgramInterface.
         *
         * @return
         */
        DebugToolDrivers::TargetInterfaces::RiscV::RiscVProgramInterface* getRiscVProgramInterface() override {
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
