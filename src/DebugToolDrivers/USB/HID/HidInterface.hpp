#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <chrono>

#include "hidapi.hpp"

namespace Bloom::Usb
{
    /**
     * The HidInterface uses the HIDAPI library to implement communication with HID endpoints.
     *
     * Currently, this interface only supports single-report HID implementations. HID interfaces with
     * multiple reports will be supported as-and-when we need it.
     */
    class HidInterface
    {
    public:
        std::uint8_t interfaceNumber = 0;

        HidInterface(
            std::uint8_t interfaceNumber,
            std::uint16_t vendorId,
            std::uint16_t productId
        );

        HidInterface(const HidInterface& other) = delete;
        HidInterface& operator = (const HidInterface& other) = delete;

        HidInterface(HidInterface&& other) = default;
        HidInterface& operator = (HidInterface&& other) = default;

        std::size_t getInputReportSize() const {
             return this->inputReportSize;
        }

        /**
         * Obtains a hid_device instance and claims the HID interface on the device.
         */
        void init();

        /**
         * Releases any claimed interfaces and closes the hid_device.
         */
        void close();

        /**
         * Reads as much data as the device has to offer, into a vector.
         *
         * @param timeout
         *
         * @return
         *  A vector of the data received from the device.
         */
        std::vector<unsigned char> read(std::optional<std::chrono::milliseconds> timeout = std::nullopt);

        /**
         * Writes buffer to HID output endpoint.
         *
         * @param buffer
         */
        void write(std::vector<unsigned char>&& buffer);

        std::string getHidDevicePath();

    private:
        using HidDevice = std::unique_ptr<::hid_device, decltype(&::hid_close)>;

        HidDevice hidDevice = HidDevice(nullptr, ::hid_close);

        /**
         * All HID reports have a fixed report length. This means that every packet we send or receive to/from an HID
         * endpoint must be equal (in size) to the report length.
         */
        std::size_t inputReportSize = 64;

        std::uint16_t vendorId = 0;
        std::uint16_t productId = 0;
    };
}
