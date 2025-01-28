#include "WchLinkBase.hpp"

#include <chrono>
#include <array>

#include "Protocols/WchLink/WchLinkInterface.hpp"

#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

#include "src/Logger/Logger.hpp"

namespace DebugToolDrivers::Wch
{
    using Exceptions::DeviceInitializationFailure;

    WchLinkBase::WchLinkBase(
        const DebugToolConfig& toolConfig,
        WchLinkVariant variant,
        std::uint16_t vendorId,
        std::uint16_t productId,
        std::uint16_t iapVendorId,
        std::uint16_t iapProductId,
        std::uint8_t wchLinkUsbInterfaceNumber
    )
        : UsbDevice(vendorId, productId)
        , toolConfig(WchLinkToolConfig{toolConfig})
        , iapVendorId(iapVendorId)
        , iapProductId(iapProductId)
        , variant(variant)
        , wchLinkUsbInterfaceNumber(wchLinkUsbInterfaceNumber)
    {}

    void WchLinkBase::init() {
        if (this->toolConfig.exitIapMode && !UsbDevice::devicePresent(this->vendorId, this->productId)) {
            auto iapDevice = Usb::UsbDevice::tryDevice(this->iapVendorId, this->iapProductId);

            if (iapDevice.has_value()) {
                Logger::warning("Found device in IAP mode - attempting exit operation");
                this->exitIapMode(*iapDevice);

                Logger::info("Waiting for device to re-enumerate...");
                if (!Usb::UsbDevice::waitForDevice(this->vendorId, this->productId, std::chrono::seconds{8})) {
                    throw DeviceInitializationFailure{"Timeout exceeded whilst waiting for device to re-enumerate"};
                }

                Logger::info("Re-enumerated device found - IAP exit operation was successful");
            }
        }

        UsbDevice::init();

        this->detachKernelDriverFromInterface(this->wchLinkUsbInterfaceNumber);

        this->wchLinkUsbInterface = std::make_unique<Usb::UsbInterface>(
            this->wchLinkUsbInterfaceNumber,
            this->libusbDeviceHandle.get()
        );

        this->wchLinkUsbInterface->init();

        this->wchLinkInterface = std::make_unique<Protocols::WchLink::WchLinkInterface>(
            *(this->wchLinkUsbInterface),
            *this
        );

        if (this->getDeviceInfo().variant != this->variant) {
            throw DeviceInitializationFailure{
                "WCH-Link variant mismatch - device returned variant ID that doesn't match the " + this->getName()
                    + " variant ID"
            };
        }

        this->initialised = true;
    }

    void WchLinkBase::close() {
        if (this->wchLinkUsbInterface) {
            this->wchLinkUsbInterface->close();
        }

        this->initialised = false;
    }

    void WchLinkBase::postInit() {
        const auto& deviceInfo = this->getDeviceInfo();

        constexpr auto MIN_VERSION = WchFirmwareVersion{.major = 2, .minor = 9};
        if (deviceInfo.firmwareVersion < MIN_VERSION) {
            Logger::warning(
                "WCH-Link firmware version (" + deviceInfo.firmwareVersion.toString() + ") may not be compatible with"
                    " Bloom. A minimum version of " + MIN_VERSION.toString() + " is required."
            );
        }

        Logger::info("WCH-Link firmware version: " + deviceInfo.firmwareVersion.toString());
    }

    bool WchLinkBase::isInitialised() const {
        return this->initialised;
    }

    std::string WchLinkBase::getSerialNumber() {
        return UsbDevice::getSerialNumber();
    }

    WchLinkDebugInterface* WchLinkBase::getRiscVDebugInterface(
        const Targets::RiscV::TargetDescriptionFile& targetDescriptionFile,
        const Targets::RiscV::RiscVTargetConfig& targetConfig
    ) {

        if (!this->wchLinkDebugInterface) {
            this->wchLinkDebugInterface = std::make_unique<WchLinkDebugInterface>(
                this->toolConfig,
                targetConfig,
                targetDescriptionFile,
                *(this->wchLinkInterface)
            );
        }
        return this->wchLinkDebugInterface.get();
    }

    const DeviceInfo& WchLinkBase::getDeviceInfo() const {
        if (!this->cachedDeviceInfo.has_value()) {
            this->cachedDeviceInfo = this->wchLinkInterface->getDeviceInfo();
        }

        return *(this->cachedDeviceInfo);
    }

    void WchLinkBase::exitIapMode(UsbDevice& iapDevice) const {
        static constexpr auto IAP_INTERFACE_NUMBER = std::uint8_t{0};
        static constexpr auto IAP_COMMAND_ENDPOINT_ADDRESS = std::uint8_t{0x02};

        auto interface = Usb::UsbInterface{IAP_INTERFACE_NUMBER, iapDevice.libusbDeviceHandle.get()};
        interface.init();

        static constexpr auto COMMAND_BUFFER = std::to_array<unsigned char>({0x83});
        interface.writeBulk(IAP_COMMAND_ENDPOINT_ADDRESS, COMMAND_BUFFER, 64);
    }
}
