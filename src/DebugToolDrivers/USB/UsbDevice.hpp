#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <optional>
#include <libusb-1.0/libusb.h>

#include "src/DebugToolDrivers/DebugTool.hpp"

namespace Bloom::Usb
{
    class UsbDevice
    {
    protected:
        libusb_context* libUsbContext = nullptr;
        libusb_device* libUsbDevice = nullptr;
        libusb_device_handle* libUsbDeviceHandle = nullptr;
        std::uint16_t vendorId;
        std::uint16_t productId;

        std::vector<libusb_device*> findMatchingDevices(
            std::optional<std::uint16_t> vendorId = std::nullopt, std::optional<std::uint16_t> productId = std::nullopt
        );

        void close();

    public:
        void init();

        UsbDevice(std::uint16_t vendorId, std::uint16_t productId) {
            this->vendorId = vendorId;
            this->productId = productId;
        };

        ~UsbDevice() = default;

        [[nodiscard]] libusb_device* getLibUsbDevice() const {
            return this->libUsbDevice;
        }

        void setLibUsbDevice(libusb_device* libUsbDevice) {
            this->libUsbDevice = libUsbDevice;
        }

        std::uint16_t getVendorId() const {
            return this->vendorId;
        }

        std::uint16_t getProductId() const {
            return this->productId;
        }

        /**
         * Selects a specific configuration on the device, using the configuration index.
         *
         * @param configIndex
         */
        virtual void setConfiguration(int configIndex);
    };
}
