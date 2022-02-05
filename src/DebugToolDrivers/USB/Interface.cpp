#include "Interface.hpp"

#include <libusb-1.0/libusb.h>

#include "src/TargetController/Exceptions/DeviceFailure.hpp"
#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

namespace Bloom::Usb
{
    using namespace Bloom::Exceptions;

    void Interface::init() {
        if (this->libUsbDevice == nullptr) {
            throw DeviceInitializationFailure("Cannot initialise interface without libusb device pointer.");
        }

        if (this->libUsbDeviceHandle == nullptr) {
            throw DeviceInitializationFailure("Cannot initialise interface without libusb device handle.");
        }

        this->initialised = true;
    }

    void Interface::close() {
        if (this->libUsbDeviceHandle != nullptr) {
            this->release();
        }

        this->initialised = false;
    }

    void Interface::claim() {
        int interfaceNumber = this->getNumber();

        this->detachKernelDriver();

        if (libusb_claim_interface(this->libUsbDeviceHandle, interfaceNumber) != 0) {
            throw DeviceInitializationFailure(
                "Failed to claim interface {" + std::to_string(interfaceNumber) + "} on USB device\n"
            );
        }

        this->claimed = true;
    }

    void Interface::detachKernelDriver() {
        int interfaceNumber = this->getNumber();
        int libUsbStatusCode;

        if ((libUsbStatusCode = libusb_kernel_driver_active(this->libUsbDeviceHandle, interfaceNumber)) != 0) {
            if (libUsbStatusCode == 1) {
                // A kernel driver is active on this interface. Attempt to detach it
                if (libusb_detach_kernel_driver(this->libUsbDeviceHandle, interfaceNumber) != 0) {
                    throw DeviceInitializationFailure("Failed to detach kernel driver from interface " +
                        std::to_string(interfaceNumber) + "\n");
                }
            } else {
                throw DeviceInitializationFailure("Failed to check for active kernel driver on USB interface.");
            }
        }
    }

    void Interface::release() {
        if (this->isClaimed()) {
            if (libusb_release_interface(this->libUsbDeviceHandle, this->getNumber()) != 0) {
                throw DeviceFailure(
                    "Failed to release interface {" + std::to_string(this->getNumber())
                        + "} on USB device\n"
                );
            }

            this->claimed = false;
        }
    }

    int Interface::read(unsigned char* buffer, unsigned char endPoint, size_t length, size_t timeout) {
        int totalTransferred = 0;
        int transferred = 0;
        int libUsbStatusCode = 0;

        while (length > totalTransferred) {
            libUsbStatusCode = libusb_interrupt_transfer(
                this->libUsbDeviceHandle,
                endPoint,
                buffer,
                static_cast<int>(length),
                &transferred,
                static_cast<unsigned int>(timeout)
            );

            if (libUsbStatusCode != 0 && libUsbStatusCode != -7) {
                throw DeviceCommunicationFailure(
                    "Failed to read from USB device. Error code returned: " + std::to_string(libUsbStatusCode)
                );
            }

            totalTransferred += transferred;
        }

        return transferred;
    }

    void Interface::write(unsigned char* buffer, unsigned char endPoint, int length) {
        int transferred = 0;
        int libUsbStatusCode = 0;

        libUsbStatusCode = libusb_interrupt_transfer(
            this->libUsbDeviceHandle,
            endPoint,
            buffer,
            length,
            &transferred,
            0
        );

        if (libUsbStatusCode != 0) {
            throw DeviceCommunicationFailure(
                "Failed to read from USB device. Error code returned: " + std::to_string(libUsbStatusCode)
            );
        }
    }
}
