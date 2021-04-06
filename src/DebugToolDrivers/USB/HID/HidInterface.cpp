#include <cstdint>
#include <string>
#include <stdexcept>

#include "HidInterface.hpp"
#include "hidapi.hpp"
#include "src/Logger/Logger.hpp"
#include "src/Exceptions/Exception.hpp"
#include "src/Exceptions/DeviceCommunicationFailure.hpp"

using namespace Bloom::Usb;
using namespace Bloom::Exceptions;

std::string HidInterface::getDevicePathByInterfaceNumber(const std::uint16_t& interfaceNumber) {
    hid_device_info* HIDDeviceInfoList = hid_enumerate(this->getVendorId(), this->getProductId());

    while (HIDDeviceInfoList != nullptr) {
        if (HIDDeviceInfoList->interface_number == interfaceNumber) {
            break;
        }

        HIDDeviceInfoList = HIDDeviceInfoList->next;
    }

    if (HIDDeviceInfoList == nullptr) {
        throw std::runtime_error("Failed to match interface number with HID interface.");
    }

    auto path = std::string(HIDDeviceInfoList->path);
    hid_free_enumeration(HIDDeviceInfoList);
    return path;
}

void HidInterface::init() {
    /*
     * Because we use the HIDAPI with libusb for our HID interfaces, we must allow the HIDAPI to own the device
     * resources (device handle and the interface). However, the HIDAPI does not provide any means to ensure that a
     * specific configuration is set against the device. This is why we first open the device via libusb (by calling
     * the generic init() method), so that we can set the correct configuration. We then close the device to allow
     * the HIDAPI to take ownership.
     */
    Interface::init();
    Interface::detachKernelDriver();
    Interface::setConfiguration(0);
    Interface::close();

    hid_init();
    hid_device* hidDevice;

    std::string HIDInterfacePath = this->getDevicePathByInterfaceNumber(this->getNumber());
    Logger::debug("HID device path: " + HIDInterfacePath);

    if ((hidDevice = hid_open_path(HIDInterfacePath.c_str())) == nullptr) {
        throw Exception("Failed to open HID device via hidapi.");
    }

    if (hidDevice->input_ep_max_packet_size < 1) {
        throw Exception("Invalid max packet size for USB endpoint, on interface " + std::to_string(this->getNumber()));
    }

    this->setHidDevice(hidDevice);
    this->setInputReportSize(static_cast<std::size_t>(hidDevice->input_ep_max_packet_size));
    this->setLibUsbDeviceHandle(hidDevice->device_handle);
}

void HidInterface::close() {
    auto hidDevice = this->getHidDevice();

    if (hidDevice != nullptr) {
        this->setLibUsbDeviceHandle(nullptr);
        hid_close(hidDevice);
        // hid_close() releases the interface
        Interface::setClaimed(false);
    }

    hid_exit();
    Interface::close();
}

std::size_t HidInterface::read(unsigned char* buffer, std::size_t maxLength, unsigned int timeout) {
    int transferred;

    if ((transferred = hid_read_timeout(this->getHidDevice(), buffer, maxLength,
        timeout == 0 ? -1 : static_cast<int>(timeout))) == -1
    ) {
        throw DeviceCommunicationFailure("Failed to read from HID device.");
    }

    return static_cast<std::size_t>(transferred);
}

void HidInterface::write(unsigned char* buffer, std::size_t length) {
    int transferred;

    if ((transferred = hid_write(this->getHidDevice(), buffer, length)) != length) {
        Logger::debug("Attempted to write " + std::to_string(length)
        + " bytes to HID interface. Bytes written: " + std::to_string(transferred));
        throw DeviceCommunicationFailure("Failed to write data to HID interface.");
    }
}

void HidInterface::write(std::vector<unsigned char> buffer) {
    if (buffer.size() > this->getInputReportSize()) {
        throw Exception("Cannot send data via HID interface - data exceeds maximum packet size.");

    } else if (buffer.size() < this->getInputReportSize()) {
        /*
         * Every report we send via the USB HID interface should be of a fixed size.
         * In the event of a report being too small, we just fill the buffer vector with 0.
         */
        buffer.resize(this->getInputReportSize(), 0);
    }

    this->write(buffer.data(), buffer.size());
}

std::vector<unsigned char> HidInterface::read(unsigned int timeout) {
    std::vector<unsigned char> output;
    auto readSize = this->getInputReportSize();

    // Attempt to read the first HID report packet, and whatever is left after that.
    output.resize(readSize);
    auto transferredByteCount = this->read(output.data(), readSize, timeout);
    auto totalByteCount = transferredByteCount;

    while (transferredByteCount >= readSize) {
        output.resize(totalByteCount + readSize);

        transferredByteCount = this->read(output.data() + totalByteCount, readSize, 1);
        totalByteCount += transferredByteCount;
    }

    output.resize(totalByteCount);
    return output;
}
