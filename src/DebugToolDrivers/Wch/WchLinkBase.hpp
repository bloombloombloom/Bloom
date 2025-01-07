#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "src/DebugToolDrivers/DebugTool.hpp"
#include "src/DebugToolDrivers/Usb/UsbDevice.hpp"
#include "src/DebugToolDrivers/Usb/UsbInterface.hpp"

#include "Protocols/WchLink/WchLinkInterface.hpp"

#include "WchLinkDebugInterface.hpp"

#include "WchLinkToolConfig.hpp"
#include "src/ProjectConfig.hpp"

#include "src/Targets/RiscV/Wch/TargetDescriptionFile.hpp"

#include "WchGeneric.hpp"
#include "DeviceInfo.hpp"

namespace DebugToolDrivers::Wch
{
    class WchLinkBase: public DebugTool, public Usb::UsbDevice
    {
    public:
        WchLinkBase(
            const DebugToolConfig& toolConfig,
            WchLinkVariant variant,
            std::uint16_t vendorId,
            std::uint16_t productId,
            std::uint16_t iapVendorId,
            std::uint16_t iapProductId,
            std::uint8_t wchLinkUsbInterfaceNumber
        );

        void init() override;

        void close() override;

        void postInit() override;

        [[nodiscard]] bool isInitialised() const override;

        std::string getSerialNumber() override;

        WchLinkDebugInterface* getRiscVDebugInterface(
            const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile,
            const Targets::RiscV::RiscVTargetConfig& targetConfig
        ) override;

    protected:
        WchLinkToolConfig toolConfig;
        std::uint16_t iapVendorId;
        std::uint16_t iapProductId;

        bool initialised = false;

        WchLinkVariant variant;

        std::uint8_t wchLinkUsbInterfaceNumber;
        std::unique_ptr<Usb::UsbInterface> wchLinkUsbInterface = nullptr;
        std::unique_ptr<Protocols::WchLink::WchLinkInterface> wchLinkInterface = nullptr;
        std::unique_ptr<WchLinkDebugInterface> wchLinkDebugInterface = nullptr;

        mutable std::optional<DeviceInfo> cachedDeviceInfo = std::nullopt;

        const DeviceInfo& getDeviceInfo() const;
        void exitIapMode(Usb::UsbDevice& iapDevice) const;
    };
}
