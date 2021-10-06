#include "UsbDevice.hpp"

#include "src/Logger/Logger.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

using Bloom::Usb::UsbDevice;
using namespace Bloom::Exceptions;

void UsbDevice::init() {
    libusb_init(&this->libUsbContext);
//    libusb_set_option(this->libUsbContext, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_NONE);
    auto devices = this->findMatchingDevices();

    if (devices.empty()) {
        throw DeviceInitializationFailure("Failed to find USB device with matching vendor & product ID.");

    } else if (devices.size() > 1) {
        // TODO: implement support for multiple devices (maybe via serial number?)
        throw DeviceInitializationFailure(
            "Numerous devices of matching vendor & product ID found.\n"
            "Please ensure that only one debug tool is connected and then try again."
        );
    }

    // For now, just use the first device found.
    auto device = devices.front();
    this->setLibUsbDevice(device);

    int libUsbStatusCode;

    // Obtain a device handle from libusb
    if ((libUsbStatusCode = libusb_open(libUsbDevice, &this->libUsbDeviceHandle)) < 0) {
        throw DeviceInitializationFailure(
            "Failed to open USB device - error code " + std::to_string(libUsbStatusCode) + " returned."
        );
    }
}

void UsbDevice::setConfiguration(int configIndex) {
    libusb_config_descriptor* configDescriptor = {};
    int libUsbStatusCode;

    if ((libUsbStatusCode = libusb_get_config_descriptor(this->libUsbDevice, 0, &configDescriptor))) {
        throw DeviceInitializationFailure(
            "Failed to obtain USB configuration descriptor - error code " + std::to_string(libUsbStatusCode)
            + " returned."
        );
    }

    if ((libUsbStatusCode = libusb_set_configuration(this->libUsbDeviceHandle, configDescriptor->bConfigurationValue))) {
        throw DeviceInitializationFailure(
            "Failed to set USB configuration - error code " + std::to_string(libUsbStatusCode) + " returned."
        );
    }

    libusb_free_config_descriptor(configDescriptor);
}

std::vector<libusb_device*> UsbDevice::findMatchingDevices(
    std::optional<std::uint16_t> vendorId, std::optional<std::uint16_t> productId
) {
    auto libUsbContext = this->libUsbContext;
    libusb_device** devices = nullptr;
    libusb_device* device;
    std::vector<libusb_device*> matchedDevices;
    ssize_t i = 0, libUsbStatusCode;

    auto vendorIdToMatch = vendorId.value_or(this->vendorId);
    auto productIdToMatch = productId.value_or(this->productId);

    if ((libUsbStatusCode = libusb_get_device_list(libUsbContext, &devices)) < 0) {
        throw DeviceInitializationFailure(
            "Failed to retrieve USB devices - return code: '" + std::to_string(libUsbStatusCode) + "'"
        );
    }

    while ((device = devices[i++]) != nullptr) {
        struct libusb_device_descriptor desc = {};

        if ((libUsbStatusCode = libusb_get_device_descriptor(device, &desc)) < 0) {
            Logger::warning("Failed to retrieve USB device descriptor - return code: '"
                + std::to_string(libUsbStatusCode) + "'");
            continue;
        }

        if (desc.idVendor == vendorIdToMatch && desc.idProduct == productIdToMatch) {
            matchedDevices.push_back(device);
        }
    }

    libusb_free_device_list(devices, 1);
    return matchedDevices;
}

void UsbDevice::close() {
    if (this->libUsbDeviceHandle != nullptr) {
        libusb_close(this->libUsbDeviceHandle);
        this->libUsbDeviceHandle = nullptr;
    }

    if (this->libUsbContext != nullptr) {
        libusb_exit(this->libUsbContext);
    }
}
