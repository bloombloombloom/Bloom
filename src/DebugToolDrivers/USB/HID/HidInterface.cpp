#include "HidInterface.hpp"

#include "src/Logger/Logger.hpp"

#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"
#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace Bloom::Usb
{
    using namespace Bloom::Exceptions;

    void HidInterface::init() {
        if (this->libUsbDevice == nullptr) {
            throw DeviceInitializationFailure("Cannot initialise interface without libusb device pointer.");
        }

        hid_init();
        hid_device* hidDevice = nullptr;

        std::string hidInterfacePath = this->getDevicePathByInterfaceNumber(this->getNumber());
        Logger::debug("HID device path: " + hidInterfacePath);

        if ((hidDevice = hid_open_path(hidInterfacePath.c_str())) == nullptr) {
            throw DeviceInitializationFailure("Failed to open HID device via hidapi.");
        }

        if (hidDevice->input_ep_max_packet_size < 1) {
            throw DeviceInitializationFailure(
                "Invalid max packet size for USB endpoint, on interface "
                    + std::to_string(this->getNumber())
            );
        }

        this->setHidDevice(hidDevice);
        this->setInputReportSize(static_cast<std::size_t>(hidDevice->input_ep_max_packet_size));
        this->libUsbDeviceHandle = hidDevice->device_handle;
        this->initialised = true;
    }

    void HidInterface::close() {
        auto* hidDevice = this->getHidDevice();

        if (hidDevice != nullptr) {
            this->libUsbDeviceHandle = nullptr;
            hid_close(hidDevice);
            // hid_close() releases the interface
            this->claimed = false;
        }

        hid_exit();
        Interface::close();
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

    void HidInterface::write(std::vector<unsigned char>&& buffer) {
        if (buffer.size() > this->getInputReportSize()) {
            throw DeviceCommunicationFailure(
                "Cannot send data via HID interface - data exceeds maximum packet size."
            );
        }

        if (buffer.size() < this->getInputReportSize()) {
            /*
             * Every report we send via the USB HID interface should be of a fixed size.
             * In the event of a report being too small, we just fill the buffer vector with 0.
             */
            buffer.resize(this->getInputReportSize(), 0);
        }

        int transferred = 0;
        const auto length = buffer.size();

        if ((transferred = hid_write(this->getHidDevice(), buffer.data(), length)) != length) {
            Logger::debug("Attempted to write " + std::to_string(length)
                + " bytes to HID interface. Bytes written: " + std::to_string(transferred));
            throw DeviceCommunicationFailure("Failed to write data to HID interface.");
        }
    }

    std::size_t HidInterface::read(unsigned char* buffer, std::size_t maxLength, unsigned int timeout) {
        int transferred;

        if ((transferred = hid_read_timeout(
                this->hidDevice,
                buffer,
                maxLength,
                timeout == 0 ? -1 : static_cast<int>(timeout))
            ) == -1
        ) {
            throw DeviceCommunicationFailure("Failed to read from HID device.");
        }

        return static_cast<std::size_t>(transferred);
    }

    std::string HidInterface::getDevicePathByInterfaceNumber(const std::uint16_t& interfaceNumber) {
        hid_device_info* hidDeviceInfoList = hid_enumerate(
            this->getVendorId(),
            this->getProductId()
        );

        while (hidDeviceInfoList != nullptr) {
            if (hidDeviceInfoList->interface_number == interfaceNumber) {
                break;
            }

            hidDeviceInfoList = hidDeviceInfoList->next;
        }

        if (hidDeviceInfoList == nullptr) {
            throw DeviceInitializationFailure("Failed to match interface number with HID interface.");
        }

        auto path = std::string(hidDeviceInfoList->path);
        hid_free_enumeration(hidDeviceInfoList);
        return path;
    }
}
