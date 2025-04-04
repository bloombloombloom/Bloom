#include "UsbDevice.hpp"

#include <libusb-1.0/libusb.h>
#include <array>
#include <thread>

#include "src/Logger/Logger.hpp"
#include "src/Services/StringService.hpp"

#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"
#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"
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

    UsbDevice::~UsbDevice() {
        this->close();
    }

    void UsbDevice::init() {
//        ::libusb_set_option(this->libusbContext, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_NONE);
        auto devices = this->findMatchingDevices(this->vendorId, this->productId);

        if (devices.empty()) {
            throw DeviceNotFound{
                "Failed to find USB device with matching vendor and product ID. Please examine the debug tool's USB "
                    "connection, as well as the selected environment's debug tool configuration, in bloom.yaml"
            };
        }

        if (devices.size() > 1) {
            // TODO: implement support for multiple devices via serial number matching?
            throw DeviceInitializationFailure{
                "Numerous devices of matching vendor and product ID found.\n"
                    "Please ensure that only one debug tool is connected and then try again."
            };
        }

        this->libusbDevice.swap(devices.front());
        ::libusb_device_handle* deviceHandle = nullptr;

        const int libusbStatusCode = ::libusb_open(this->libusbDevice.get(), &deviceHandle);

        if (libusbStatusCode < 0) {
            throw DeviceInitializationFailure{
                "Failed to open USB device - error code " + std::to_string(libusbStatusCode) + " returned."
            };
        }

        this->libusbDeviceHandle.reset(deviceHandle);
    }

    std::string UsbDevice::getSerialNumber() const {
        assert(this->libusbDevice && this->libusbDeviceHandle);
        struct ::libusb_device_descriptor desc = {};

        auto statusCode = ::libusb_get_device_descriptor(this->libusbDevice.get(), &desc);
        if (statusCode != 0) {
            throw DeviceCommunicationFailure{
                "Failed to retrieve USB device descriptor - status code: " + std::to_string(statusCode)
            };
        }

        auto data = std::array<unsigned char, 256>{};
        const auto transferredBytes = ::libusb_get_string_descriptor_ascii(
            this->libusbDeviceHandle.get(),
            desc.iSerialNumber,
            data.data(),
            data.size()
        );

        if (transferredBytes <= 0) {
            throw DeviceCommunicationFailure{
                "Failed to retrieve serial number from USB device - status code: " + std::to_string(transferredBytes)
            };
        }

        return {data.begin(), data.begin() + transferredBytes};
    }

    void UsbDevice::setConfiguration(std::uint8_t configurationIndex) {
        const auto configDescriptor = this->getConfigDescriptor(configurationIndex);

        const auto libusbStatusCode = ::libusb_set_configuration(
            this->libusbDeviceHandle.get(),
            configDescriptor->bConfigurationValue
        );

        if (libusbStatusCode < 0) {
            throw DeviceInitializationFailure{
                "Failed to set USB configuration - error code " + std::to_string(libusbStatusCode) + " returned."
            };
        }
    }

    std::optional<UsbDevice> UsbDevice::tryDevice(std::uint16_t vendorId, std::uint16_t productId) {
        auto device = UsbDevice{vendorId, productId};

        try {
            device.init();

        } catch (const DeviceNotFound&) {
            return std::nullopt;
        }

        return device;
    }

    bool UsbDevice::devicePresent(std::uint16_t vendorId, std::uint16_t productId) {
        return !UsbDevice::findMatchingDevices(vendorId, productId).empty();
    }

    bool UsbDevice::waitForDevice(std::uint16_t vendorId, std::uint16_t productId, std::chrono::milliseconds timeout) {
        static constexpr auto DELAY = std::chrono::milliseconds{50};
        for (auto i = 0; (i * DELAY) <= timeout; ++i){
            if (!UsbDevice::findMatchingDevices(vendorId, productId).empty()) {
                return true;
            }

            std::this_thread::sleep_for(DELAY);
        }

        return false;
    }

    std::uint8_t UsbDevice::getFirstEndpointAddress(
        std::uint8_t interfaceNumber,
        ::libusb_endpoint_direction direction
    ) {
        const auto activeConfigDescriptor = this->getConfigDescriptor();

        for (auto interfaceIndex = 0; interfaceIndex < activeConfigDescriptor->bNumInterfaces; ++interfaceIndex) {
            const auto* interfaceDescriptor = (activeConfigDescriptor->interface + interfaceIndex)->altsetting;

            if (interfaceDescriptor->bInterfaceNumber != interfaceNumber) {
                continue;
            }

            for (auto endpointIndex = 0; endpointIndex < interfaceDescriptor->bNumEndpoints; ++endpointIndex) {
                const auto* endpointDescriptor = (interfaceDescriptor->endpoint + endpointIndex);

                if ((endpointDescriptor->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) != direction) {
                    return endpointDescriptor->bEndpointAddress;
                }
            }
        }

        throw DeviceInitializationFailure{"Failed to obtain address of USB endpoint"};
    }

    std::uint16_t UsbDevice::getEndpointMaxPacketSize(std::uint8_t endpointAddress) {
        const auto activeConfigDescriptor = this->getConfigDescriptor();

        for (auto interfaceIndex = 0; interfaceIndex < activeConfigDescriptor->bNumInterfaces; ++interfaceIndex) {
            const auto* interfaceDescriptor = (activeConfigDescriptor->interface + interfaceIndex)->altsetting;

            for (auto endpointIndex = 0; endpointIndex < interfaceDescriptor->bNumEndpoints; ++endpointIndex) {
                const auto* endpointDescriptor = (interfaceDescriptor->endpoint + endpointIndex);

                if (endpointDescriptor->bEndpointAddress == endpointAddress) {
                    return endpointDescriptor->wMaxPacketSize;
                }
            }
        }

        throw DeviceInitializationFailure{
            "Failed to obtain maximum packet size of USB endpoint (address: 0x"
                + Services::StringService::toHex(endpointAddress) + "). Endpoint not found. Selected configuration "
                "value (" + std::to_string(activeConfigDescriptor->bConfigurationValue) + ")"
        };
    }

    std::vector<LibusbDevice> UsbDevice::findMatchingDevices(std::uint16_t vendorId, std::uint16_t productId) {
        ::libusb_device** devices = nullptr;
        ::libusb_device* device;
        auto matchedDevices = std::vector<LibusbDevice>{};

        auto libusbStatusCode = ::libusb_get_device_list(UsbDevice::libusbContext.get(), &devices);
        if (libusbStatusCode < 0) {
            throw DeviceInitializationFailure{
                "Failed to retrieve USB devices - return code: '" + std::to_string(libusbStatusCode) + "'"
            };
        }

        ssize_t i = 0;
        while ((device = devices[i++]) != nullptr) {
            auto libusbDevice = LibusbDevice{device, ::libusb_unref_device};
            struct ::libusb_device_descriptor desc = {};

            if ((libusbStatusCode = ::libusb_get_device_descriptor(device, &desc)) < 0) {
                Logger::warning(
                    "Failed to retrieve USB device descriptor - return code: '"
                        + std::to_string(libusbStatusCode) + "'"
                );
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
            throw DeviceInitializationFailure{
                "Failed to obtain USB configuration descriptor - error code " + std::to_string(libusbStatusCode)
                    + " returned."
            };
        }

        return {configDescriptor, ::libusb_free_config_descriptor};
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
            throw DeviceInitializationFailure{"Failed to check for active kernel driver on USB interface."};
        }
    }

    void UsbDevice::close() {
        this->libusbDeviceHandle.reset();
        this->libusbDevice.reset();
    }
}
