#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <optional>
#include <libusb-1.0/libusb.h>

#include "src/DebugToolDrivers/DebugTool.hpp"

namespace Bloom::Usb
{
    using LibusbContextType = std::unique_ptr<::libusb_context, decltype(&::libusb_exit)>;
    using LibusbDeviceType = std::unique_ptr<::libusb_device, decltype(&::libusb_unref_device)>;
    using LibusbDeviceHandleType = std::unique_ptr<::libusb_device_handle, decltype(&::libusb_close)>;

    class UsbDevice
    {
    public:
        std::uint16_t vendorId;
        std::uint16_t productId;

        UsbDevice(std::uint16_t vendorId, std::uint16_t productId);

        UsbDevice(const UsbDevice& other) = delete;
        UsbDevice& operator = (const UsbDevice& other) = delete;

        UsbDevice(UsbDevice&& other) = default;
        UsbDevice& operator = (UsbDevice&& other) = default;

        void init();

        /**
         * Selects a specific configuration on the device, using the configuration index.
         *
         * @param configIndex
         */
        virtual void setConfiguration(int configIndex);

        virtual ~UsbDevice();

    protected:
        static inline LibusbContextType libusbContext = LibusbContextType(nullptr, ::libusb_exit);

        LibusbDeviceType libusbDevice = LibusbDeviceType(nullptr, ::libusb_unref_device);
        LibusbDeviceHandleType libusbDeviceHandle = LibusbDeviceHandleType(nullptr, ::libusb_close);

        std::vector<LibusbDeviceType> findMatchingDevices(std::uint16_t vendorId, std::uint16_t productId);

        void detachKernelDriverFromInterface(std::uint8_t interfaceNumber);

        void close();
    };
}
