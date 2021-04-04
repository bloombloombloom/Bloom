#include <cstdint>
#include <filesystem>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/usbdevice_fs.h>
#include <libusb-1.0/libusb.h>

#include "UsbDevice.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"

using Bloom::Usb::UsbDevice;
using namespace Bloom::Exceptions;

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
        throw Exception("Failed to retrieve USB devices - return code: '" + std::to_string(libUsbStatusCode)
            + "'");
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

void UsbDevice::init()
{
    libusb_init(&this->libUsbContext);
//    libusb_set_option(this->libUsbContext, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_NONE);
    auto devices = this->findMatchingDevices();

    if (devices.empty()) {
        throw Exception("Failed to find USB device with matching vendor & product ID.");

    } else if (devices.size() > 1) {
        // TODO: implement support for multiple devices (maybe via serial number?)
        throw Exception("Multiple devices of matching vendor & product ID found.\n"
                                 "Yes, as a program I really am too stupid to figure out what to do "
                                 "here, so I'm just going to quit.\n Please ensure that only one debug tool "
                                 "is connected and then try again.");
    }

    // For now, just use the first device found.
    this->setLibUsbDevice(devices.front());
}

void UsbDevice::close()
{
    if (this->libUsbContext != nullptr) {
        libusb_exit(this->libUsbContext);
    }
}
