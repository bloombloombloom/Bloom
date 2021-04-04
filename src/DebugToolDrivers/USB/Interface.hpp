#pragma once

#include <cstdint>
#include <libusb-1.0/libusb.h>
#include <string>

#include "UsbDevice.hpp"

namespace Bloom::Usb
{
    class Interface
    {
    private:
        libusb_device* USBDevice = nullptr;
        std::uint16_t vendorId = 0;
        std::uint16_t productId = 0;

        /**
         * With libusb, we can only claim a single USB interface per device handle. For this reason,
         * device handles are stored against the interface. Each interface must obtain a new handle before
         * claiming.
         */
        libusb_device_handle* libUsbDeviceHandle = nullptr;
        std::uint8_t number = 0;
        std::string name = "";
        bool initialised = false;
        bool claimed = false;

        void setInitialised(bool initialised) {
            this->initialised = initialised;
        }

    protected:
        void setClaimed(bool claimed) {
            this->claimed = claimed;
        }

    public:
        explicit Interface(const std::uint8_t& interfaceNumber = 0) {
            this->setNumber(interfaceNumber);
        }

        libusb_device* getUSBDevice() const {
            return this->USBDevice;
        }

        void setUSBDevice(libusb_device* USBDevice) {
            this->USBDevice = USBDevice;
        }

        libusb_device_handle* getLibUsbDeviceHandle() const {
            return this->libUsbDeviceHandle;
        }

        void setLibUsbDeviceHandle(libusb_device_handle* libUsbDeviceHandle) {
            this->libUsbDeviceHandle = libUsbDeviceHandle;
        }

        std::uint8_t getNumber() const {
            return this->number;
        }

        void setNumber(std::uint8_t number) {
            this->number = number;
        }

        const std::string& getName() const {
            return this->name;
        }

        void setName(const std::string& name) {
            this->name = name;
        }

        bool isClaimed() {
            return this->claimed;
        }

        bool isInitialised() {
            return this->initialised;
        }

        std::uint16_t getVendorId() const {
            return this->vendorId;
        }

        void setVendorId(std::uint16_t vendorId) {
            this->vendorId = vendorId;
        }

        std::uint16_t getProductId() const {
            return this->productId;
        }

        void setProductId(std::uint16_t productId) {
            this->productId = productId;
        }

        /**
         * Attempts to obtain a device descriptor and device handle via libusb.
         */
        virtual void init();

        virtual void setConfiguration(int configIndex);

        /**
         * Releases the interface and closes the device descriptor.
         */
        virtual void close();

        /**
         * Attempts to claim the interface
         */
        void claim();
        void detachKernelDriver();
        void release();

        /**
         * @TODO Remove or refactor these (read() and write()) - they're not currently used
         *
         * @param buffer
         * @param endPoint
         * @param length
         * @param timeout
         * @return
         */
        virtual int read(unsigned char* buffer, unsigned char endPoint, std::size_t length, std::size_t timeout);
        virtual void write(unsigned char* buffer, unsigned char endPoint, int length);
    };
}
