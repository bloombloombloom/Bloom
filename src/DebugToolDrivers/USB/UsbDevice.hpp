#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <optional>
#include <libusb-1.0/libusb.h>

#include "src/DebugToolDrivers/DebugTool.hpp"

namespace Usb
{
    using LibusbContext = std::unique_ptr<::libusb_context, decltype(&::libusb_exit)>;
    using LibusbDevice = std::unique_ptr<::libusb_device, decltype(&::libusb_unref_device)>;
    using LibusbDeviceHandle = std::unique_ptr<::libusb_device_handle, decltype(&::libusb_close)>;
    using LibusbConfigDescriptor = std::unique_ptr<::libusb_config_descriptor, decltype(&::libusb_free_config_descriptor)>;

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
         * Retrieves the device's serial number via the device descriptor.
         *
         * @return
         */
        std::string getSerialNumber() const;

        /**
         * Obtains the address of the first endpoint within a given interface and of a particular direction.
         *
         * @param endpointAddress
         * @return
         */
        std::uint8_t getFirstEndpointAddress(std::uint8_t interfaceNumber, ::libusb_endpoint_direction direction);

        /**
         * Obtains the maximum packet size of an endpoint.
         *
         * @param endpointAddress
         * @return
         */
        std::uint16_t getEndpointMaxPacketSize(std::uint8_t endpointAddress);

        /**
         * Selects a specific configuration on the device, using the configuration index.
         *
         * @param configurationIndex
         */
        virtual void setConfiguration(std::uint8_t configurationIndex);

        virtual ~UsbDevice();

    protected:
        static inline LibusbContext libusbContext = LibusbContext(nullptr, ::libusb_exit);

        LibusbDevice libusbDevice = LibusbDevice(nullptr, ::libusb_unref_device);
        LibusbDeviceHandle libusbDeviceHandle = LibusbDeviceHandle(nullptr, ::libusb_close);

        std::vector<LibusbDevice> findMatchingDevices(std::uint16_t vendorId, std::uint16_t productId);

        LibusbConfigDescriptor getConfigDescriptor(std::optional<std::uint8_t> configurationIndex = std::nullopt);

        void detachKernelDriverFromInterface(std::uint8_t interfaceNumber);

        void close();
    };
}
