#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "hidapi.hpp"
#include "src/DebugToolDrivers/USB/Interface.hpp"

namespace Bloom::Usb
{
    /**
     * The HIDInterface uses the HIDAPI library to implement communication with HID endpoints.
     *
     * Currently, this interface only supports single-report HID implementations. HID interfaces with
     * multiple reports will be supported as-and-when we need it.
     */
    class HidInterface: public Interface
    {
    private:
        /**
         * The HIDAPI library provides a hid_device data structure to represent a USB HID interface.
         *
         * @see hidapi.hpp or the HIDAPI documentation for more on this.
         */
        hid_device* hidDevice = nullptr;

        /**
         * All HID reports have a fixed report length. This means that every packet
         * we send or receive to/from an HID endpoint must be equal to the report length in size.
         *
         * The default input report size is 64 bytes, but this is overridden at interface initialisation.
         * @see definition of init() for more on this.
         */
        std::size_t inputReportSize = 64;

        void setHidDevice(hid_device* hidDevice) {
            this->hidDevice = hidDevice;
        }

        void setInputReportSize(const std::size_t& inputReportSize) {
            this->inputReportSize = inputReportSize;
        }

        /**
         * Reads a maximum of `maxLength` bytes into `buffer`, from the HID input endpoint.
         *
         * Keeping this in the private scope to enforce use of vector<unsigned char>. See read()
         * method in public scope.
         *
         * @param buffer
         * @param maxLength
         * @param timeout
         *
         * @return
         *  Number of bytes read.
         */
        std::size_t read(unsigned char* buffer, std::size_t maxLength, unsigned int timeout);

        /**
         * Writes `length` bytes from `buffer` to HID output endpoint.
         *
         * Keeping this in the private scope to enforce use of vector<unsigned char>.
         * @see write(std::vector<unsigned char> buffer);
         *
         * @param buffer
         * @param length
         */
        void write(unsigned char* buffer, std::size_t length);

    protected:
        hid_device* getHidDevice() const {
            return this->hidDevice;
        }

    public:
        std::size_t getInputReportSize() {
            return 512;
            // return this->inputReportSize;
        }

        /**
         * Claims the USB HID interface, obtains a hid_device instance and configures
         */
        void init() override;

        void close() override;

        /**
         * Wrapper for write(unsigned char *buffer, int length)
         *
         * @param buffer
         */
        void write(std::vector<unsigned char> buffer);

        /**
         * Reads as much data as the device has to offer, into a vector.
         *
         * If `timeout` is set to 0, this method will block until at least one HID report
         * packet is received.
         *
         * @param timeout
         *
         * @return
         *  A vector of the data received from the device.
         */
        std::vector<unsigned char> read(unsigned int timeout = 0);

        /**
         * Resolves a device path from a USB interface number.
         *
         * @param interfaceNumber
         * @return
         */
        std::string getDevicePathByInterfaceNumber(const std::uint16_t& interfaceNumber);
    };
}
