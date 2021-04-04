#include <libusb-1.0/libusb.h>
#include <chrono>
#include <thread>

#include "Interface.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

using namespace Bloom::Usb;
using namespace Bloom::Exceptions;


void Interface::init() {
    libusb_device_descriptor deviceDescriptor = {};
    auto deviceHandle = this->getLibUsbDeviceHandle();
    auto libUsbDevice = this->getUSBDevice();
    int libUsbStatusCode;

    if (libUsbDevice == nullptr) {
        throw Exception("Cannot open USB device without libusb device pointer.");
    }

    if (deviceHandle == nullptr) {
        // Obtain a device handle from libusb
        if ((libUsbStatusCode = libusb_open(libUsbDevice, &deviceHandle)) < 0) {
            throw Exception("Failed to open USB device - error code "
                                         + std::to_string(libUsbStatusCode) + " returned.");
        }
    }

    if ((libUsbStatusCode = libusb_get_device_descriptor(libUsbDevice, &deviceDescriptor))) {
        throw Exception("Failed to obtain USB device descriptor - error code "
                                     + std::to_string(libUsbStatusCode) + " returned.");
    }

    this->setLibUsbDeviceHandle(deviceHandle);
    this->setInitialised(true);
}

void Interface::setConfiguration(int configIndex) {
    libusb_config_descriptor* configDescriptor = {};
    int libUsbStatusCode;

    if ((libUsbStatusCode = libusb_get_config_descriptor(this->getUSBDevice(), 0, &configDescriptor))) {
        throw Exception("Failed to obtain USB configuration descriptor - error code "
                            + std::to_string(libUsbStatusCode) + " returned.");
    }

    if ((libUsbStatusCode = libusb_set_configuration(this->getLibUsbDeviceHandle(), configDescriptor->bConfigurationValue))) {
        throw Exception("Failed to set USB configuration - error code "
                            + std::to_string(libUsbStatusCode) + " returned.");
    }

    libusb_free_config_descriptor(configDescriptor);
}

void Interface::close() {
    auto deviceHandle = this->getLibUsbDeviceHandle();
    this->release();

    if (deviceHandle != nullptr) {
        libusb_close(deviceHandle);
        this->setLibUsbDeviceHandle(nullptr);
    }

    this->setInitialised(false);
}

void Interface::claim() {
    int interfaceNumber = this->getNumber();
    int libUsbStatusCode = 0;

    this->detachKernelDriver();

    if (libusb_claim_interface(this->getLibUsbDeviceHandle(), interfaceNumber) != 0) {
        throw Exception("Failed to claim interface {" + std::to_string(interfaceNumber) + "} on USB device\n");
    }

    this->setClaimed(true);
}

void Interface::detachKernelDriver() {
    int interfaceNumber = this->getNumber();
    int libUsbStatusCode;

    if ((libUsbStatusCode = libusb_kernel_driver_active(this->getLibUsbDeviceHandle(), interfaceNumber)) != 0) {
        if (libUsbStatusCode == 1) {
            // A kernel driver is active on this interface. Attempt to detach it
            if (libusb_detach_kernel_driver(this->getLibUsbDeviceHandle(), interfaceNumber) != 0) {
                throw Exception("Failed to detach kernel driver from interface " +
                    std::to_string(interfaceNumber) + "\n");
            }
        } else {
            throw Exception("Failed to check for active kernel driver on USB interface.");
        }
    }
}

void Interface::release() {
    if (this->isClaimed()) {
        if (libusb_release_interface(this->getLibUsbDeviceHandle(), this->getNumber()) != 0) {
            throw Exception("Failed to release interface {" + std::to_string(this->getNumber()) + "} on USB device\n");
        }

        this->setClaimed(false);
    }
}

int Interface::read(unsigned char* buffer, unsigned char endPoint, size_t length, size_t timeout) {
    int totalTransferred = 0;
    int transferred = 0;
    int libUsbStatusCode = 0;

    while (length > totalTransferred) {
        libUsbStatusCode = libusb_interrupt_transfer(
                this->getLibUsbDeviceHandle(),
                endPoint,
                buffer,
                static_cast<int>(length),
                &transferred,
                static_cast<unsigned int>(timeout)
        );

        if (libUsbStatusCode != 0 && libUsbStatusCode != -7) {
            throw Exception("Failed to read from USB device. Error code returned: " + std::to_string(libUsbStatusCode));
        }

        totalTransferred += transferred;
    }

    return transferred;
}

void Interface::write(unsigned char* buffer, unsigned char endPoint, int length) {
    int transferred = 0;
    int libUsbStatusCode = 0;

    libUsbStatusCode = libusb_interrupt_transfer(
        this->getLibUsbDeviceHandle(),
        endPoint,
        buffer,
        length,
        &transferred,
        0
    );

    if (libUsbStatusCode != 0) {
        throw Exception("Failed to read from USB device. Error code returned: " + std::to_string(libUsbStatusCode));
    }
}
