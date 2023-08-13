#include "UsbDevice.hpp"

#include <libusb-1.0/libusb.h>

#include "src/Logger/Logger.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"
#include "src/TargetController/Exceptions/DeviceNotFound.hpp"

namespace Usb
{
    using namespace Exceptions;

    UsbDevice::UsbDevice(std::uint16_t vendorId, std::uint16_t productId)
        : vendorId(vendorId)
        , productId(productId)
    {
        if (!UsbDevice::libusbContext) {
            ::libusb_context* libusbContext = nullptr;
            ::libusb_init(&libusbContext);
            UsbDevice::libusbContext.reset(libusbContext);
        }
    }

    void UsbDevice::init() {
//        ::libusb_set_option(this->libusbContext, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_NONE);
        auto devices = this->findMatchingDevices(this->vendorId, this->productId);

        if (devices.empty()) {
            throw DeviceNotFound(
                "Failed to find USB device with matching vendor and product ID. Please examine the debug tool's USB "
                "connection, as well as the selected environment's debug tool configuration, in bloom.yaml"
            );
        }

        if (devices.size() > 1) {
            // TODO: implement support for multiple devices via serial number matching?
            throw DeviceInitializationFailure(
                "Numerous devices of matching vendor and product ID found.\n"
                "Please ensure that only one debug tool is connected and then try again."
            );
        }

        // For now, just use the first device found.
        this->libusbDevice.swap(devices.front());
        ::libusb_device_handle* deviceHandle = nullptr;

        const int libusbStatusCode = ::libusb_open(this->libusbDevice.get(), &deviceHandle);

        if (libusbStatusCode < 0) {
            // Failed to a device handle from libusb
            throw DeviceInitializationFailure(
                "Failed to open USB device - error code " + std::to_string(libusbStatusCode) + " returned."
            );
        }

        this->libusbDeviceHandle.reset(deviceHandle);
    }

    void UsbDevice::setConfiguration(std::uint8_t configurationIndex) {
        const auto configDescriptor = this->getConfigDescriptor(configurationIndex);

        const auto libusbStatusCode = ::libusb_set_configuration(
            this->libusbDeviceHandle.get(),
            configDescriptor->bConfigurationValue
        );

        if (libusbStatusCode < 0) {
            throw DeviceInitializationFailure(
                "Failed to set USB configuration - error code " + std::to_string(libusbStatusCode) + " returned."
            );
        }
    }

    std::vector<LibusbDevice> UsbDevice::findMatchingDevices(std::uint16_t vendorId, std::uint16_t productId) {
        ::libusb_device** devices = nullptr;
        ::libusb_device* device;
        std::vector<LibusbDevice> matchedDevices;

        auto libusbStatusCode = ::libusb_get_device_list(UsbDevice::libusbContext.get(), &devices);
        if (libusbStatusCode < 0) {
            throw DeviceInitializationFailure(
                "Failed to retrieve USB devices - return code: '" + std::to_string(libusbStatusCode) + "'"
            );
        }

        ssize_t i = 0;
        while ((device = devices[i++]) != nullptr) {
            auto libusbDevice = LibusbDevice(device, ::libusb_unref_device);
            struct ::libusb_device_descriptor desc = {};

            if ((libusbStatusCode = ::libusb_get_device_descriptor(device, &desc)) < 0) {
                Logger::warning("Failed to retrieve USB device descriptor - return code: '"
                    + std::to_string(libusbStatusCode) + "'");
                continue;
            }

            if (desc.idVendor != vendorId || desc.idProduct != productId) {
                continue;
            }

            matchedDevices.emplace_back(std::move(libusbDevice));
        }

        ::libusb_free_device_list(devices, 0);
        return matchedDevices;
    }

    LibusbConfigDescriptor UsbDevice::getConfigDescriptor(std::optional<std::uint8_t> configurationIndex) {
        ::libusb_config_descriptor* configDescriptor = {};

        auto libusbStatusCode = configurationIndex.has_value()
            ? ::libusb_get_config_descriptor(this->libusbDevice.get(), *configurationIndex, &configDescriptor)
            : ::libusb_get_active_config_descriptor(this->libusbDevice.get(), &configDescriptor);

        if (libusbStatusCode < 0) {
            throw DeviceInitializationFailure(
                "Failed to obtain USB configuration descriptor - error code " + std::to_string(libusbStatusCode)
                    + " returned."
            );
        }

        return LibusbConfigDescriptor(
            configDescriptor,
            ::libusb_free_config_descriptor
        );
    }

    void UsbDevice::detachKernelDriverFromInterface(std::uint8_t interfaceNumber) {
        const auto libusbStatusCode = ::libusb_kernel_driver_active(this->libusbDeviceHandle.get(), interfaceNumber);

        if (libusbStatusCode == 1) {
            // A kernel driver is active on this interface. Attempt to detach it
            if (::libusb_detach_kernel_driver(this->libusbDeviceHandle.get(), interfaceNumber) != 0) {
                throw DeviceInitializationFailure("Failed to detach kernel driver from interface " +
                    std::to_string(interfaceNumber) + "\n");
            }

        } else if (libusbStatusCode != 0) {
            throw DeviceInitializationFailure("Failed to check for active kernel driver on USB interface.");
        }
    }

    void UsbDevice::close() {
        this->libusbDeviceHandle.reset();
        this->libusbDevice.reset();
    }

    UsbDevice::~UsbDevice() {
        this->close();
    }
}
