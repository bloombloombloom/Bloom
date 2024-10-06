#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "src/DebugToolDrivers/DebugTool.hpp"
#include "src/DebugToolDrivers/USB/UsbDevice.hpp"
#include "src/DebugToolDrivers/USB/UsbInterface.hpp"

#include "WchLinkToolConfig.hpp"
#include "src/ProjectConfig.hpp"

#include "Protocols/WchLink/WchLinkInterface.hpp"
#include "src/DebugToolDrivers/Protocols/RiscVDebugSpec/DebugTranslator.hpp"

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
            std::uint8_t wchLinkUsbInterfaceNumber
        );

        void init() override;

        void close() override;

        void postInit() override;

        [[nodiscard]] bool isInitialised() const override;

        std::string getSerialNumber() override;

        ::DebugToolDrivers::Protocols::RiscVDebugSpec::DebugTranslator* getRiscVDebugInterface(
            const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile,
            const Targets::RiscV::RiscVTargetConfig& targetConfig
        ) override;

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
         * @return
         */
        Protocols::WchLink::WchLinkInterface* getRiscVProgramInterface(
            const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile,
            const Targets::RiscV::RiscVTargetConfig& targetConfig
        ) override;

        Protocols::WchLink::WchLinkInterface* getRiscVIdentificationInterface(
            const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile,
            const Targets::RiscV::RiscVTargetConfig& targetConfig
        ) override;

    protected:
        WchLinkToolConfig toolConfig;
        bool initialised = false;

        WchLinkVariant variant;

        std::uint8_t wchLinkUsbInterfaceNumber;
        std::unique_ptr<Usb::UsbInterface> wchLinkUsbInterface = nullptr;
        std::unique_ptr<Protocols::WchLink::WchLinkInterface> wchLinkInterface = nullptr;
        std::unique_ptr<::DebugToolDrivers::Protocols::RiscVDebugSpec::DebugTranslator> wchRiscVTranslator = nullptr;

        mutable std::optional<DeviceInfo> cachedDeviceInfo = std::nullopt;

        const DeviceInfo& getDeviceInfo() const;
    };
}
