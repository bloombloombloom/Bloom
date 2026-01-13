#include "MplabPickit4.hpp"

#include "src/Logger/Logger.hpp"

#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

namespace DebugToolDrivers::Microchip
{
    MplabPickit4::MplabPickit4(const DebugToolConfig& debugToolConfig)
        : EdbgDevice(
            debugToolConfig,
            MplabPickit4::USB_VENDOR_ID,
            MplabPickit4::USB_PRODUCT_ID,
            MplabPickit4::CMSIS_HID_INTERFACE_NUMBER
        )
    {}

    void MplabPickit4::init() {
        using Exceptions::DeviceInitializationFailure;

        if (!UsbDevice::devicePresent(this->vendorId, this->productId)) {
            auto blDevice = Usb::UsbDevice::tryDevice(
                MplabPickit4::PIC_MODE_USB_VENDOR_ID,
                MplabPickit4::BL_MODE_USB_PRODUCT_ID
            );

            if (blDevice.has_value()) {
                if (!this->toolConfig.exitBootloaderMode) {
                    throw DeviceInitializationFailure{"Device is currently in bootloader mode"};
                }

                Logger::warning("Found device in bootloader mode - attempting exit operation");
                this->exitBootloaderMode(*blDevice);

                Logger::info("Waiting for device to re-enumerate...");
                if (
                    !Usb::UsbDevice::waitForDevice(
                        MplabPickit4::PIC_MODE_USB_VENDOR_ID,
                        MplabPickit4::PIC_MODE_USB_PRODUCT_ID,
                        std::chrono::seconds{8}
                    )
                ) {
                    throw DeviceInitializationFailure{"Timeout exceeded whilst waiting for device to re-enumerate"};
                }

                Logger::info("Re-enumerated device found - exit operation was successful");
            }

            auto picDevice = Usb::UsbDevice::tryDevice(
                MplabPickit4::PIC_MODE_USB_VENDOR_ID,
                MplabPickit4::PIC_MODE_USB_PRODUCT_ID
            );

            if (picDevice.has_value()) {
                if (!this->toolConfig.enableEdbgMode) {
                    throw DeviceInitializationFailure{"Device is currently in PIC mode"};
                }

                Logger::warning("Found device in PIC mode - attempting to switch to EDBG (AVR) mode");
                this->enableEdbgMode(*picDevice);

                Logger::info("Waiting for device to re-enumerate...");
                if (!Usb::UsbDevice::waitForDevice(this->vendorId, this->productId, std::chrono::seconds{8})) {
                    throw DeviceInitializationFailure{"Timeout exceeded whilst waiting for device to re-enumerate"};
                }

                Logger::info("Re-enumerated device found - mode switch operation was successful");
            }
        }

        EdbgDevice::init();
    }
}
