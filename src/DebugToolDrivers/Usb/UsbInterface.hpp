#pragma once

#include <cstdint>
#include <vector>
#include <span>
#include <optional>
#include <chrono>

#include <libusb-1.0/libusb.h>

namespace Usb
{
    /**
     * The UsbInterface provides access to a particular USB interface.
     */
    class UsbInterface
    {
    public:
        std::uint8_t interfaceNumber = 0;

        UsbInterface(
            std::uint8_t interfaceNumber,
            ::libusb_device_handle* deviceHandle
        );

        ~UsbInterface();

        UsbInterface(const UsbInterface& other) = delete;
        UsbInterface& operator = (const UsbInterface& other) = delete;

        UsbInterface(UsbInterface&& other) = default;
        UsbInterface& operator = (UsbInterface&& other) = default;

        /**
         * Attempts to claim the interface
         */
        void init();

        /**
         * Releases the claimed interface
         */
        void close();

        std::vector<unsigned char> readBulk(
            std::uint8_t endpointAddress,
            std::optional<std::chrono::milliseconds> timeout = std::nullopt
        );

        void writeBulk(
            std::uint8_t endpointAddress,
            std::span<const unsigned char> buffer,
            std::uint16_t maxPacketSize
        );

    private:
        ::libusb_device_handle* deviceHandle;
        bool claimed = false;
    };
}
