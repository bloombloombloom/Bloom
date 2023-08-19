#include "HidInterface.hpp"

#include "src/Logger/Logger.hpp"

#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"
#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace Usb
{
    using namespace Exceptions;

    HidInterface::HidInterface(
        std::uint8_t interfaceNumber,
        std::uint16_t inputReportSize,
        std::uint16_t vendorId,
        std::uint16_t productId
    )
        : interfaceNumber(interfaceNumber)
        , inputReportSize(inputReportSize)
        , vendorId(vendorId)
        , productId(productId)
    {}

    void HidInterface::init() {
        ::hid_init();
        ::hid_device* hidDevice = nullptr;

        const auto hidInterfacePath = this->getHidDevicePath();
        Logger::debug("HID device path: " + hidInterfacePath);

        if ((hidDevice = ::hid_open_path(hidInterfacePath.c_str())) == nullptr) {
            throw DeviceInitializationFailure("Failed to open HID device via hidapi.");
        }

        this->hidDevice.reset(hidDevice);
    }

    void HidInterface::close() {
        this->hidDevice.reset();
        ::hid_exit();
    }

    std::vector<unsigned char> HidInterface::read(std::optional<std::chrono::milliseconds> timeout) {
        auto output = std::vector<unsigned char>();

        const auto readSize = this->inputReportSize;
        auto transferredByteCount = int(0);
        auto totalByteCount = std::size_t(0);

        do {
            output.resize(totalByteCount + readSize, 0x00);

            transferredByteCount = ::hid_read_timeout(
                this->hidDevice.get(),
                output.data() + totalByteCount,
                readSize,
                timeout.has_value() ? static_cast<int>(timeout->count()) : -1
            );

            if (transferredByteCount == -1) {
                throw DeviceCommunicationFailure("Failed to read from HID device.");
            }

            if (totalByteCount == 0) {
                // After the first read, set the timeout to 1 millisecond, as we don't want to wait for subsequent data
                timeout = std::chrono::milliseconds(1);
            }

            totalByteCount += static_cast<std::size_t>(transferredByteCount);

        } while (transferredByteCount >= readSize);

        output.resize(totalByteCount, 0x00);
        return output;
    }

    void HidInterface::write(std::vector<unsigned char>&& buffer) {
        if (buffer.size() > this->inputReportSize) {
            throw DeviceCommunicationFailure(
                "Cannot send data via HID interface - data exceeds maximum packet size."
            );
        }

        if (buffer.size() < this->inputReportSize) {
            /*
             * Every report we send via the USB HID interface should be of a fixed size.
             * In the event of a report being too small, we just fill the buffer vector with 0.
             */
            buffer.resize(this->inputReportSize, 0);
        }

        int transferred = 0;
        const auto length = buffer.size();

        if ((transferred = ::hid_write(this->hidDevice.get(), buffer.data(), length)) != length) {
            Logger::debug("Attempted to write " + std::to_string(length)
                + " bytes to HID interface. Bytes written: " + std::to_string(transferred));
            throw DeviceCommunicationFailure("Failed to write data to HID interface.");
        }
    }

    std::string HidInterface::getHidDevicePath() {
        const auto hidDeviceInfoList = std::unique_ptr<::hid_device_info, decltype(&::hid_free_enumeration)>(
            ::hid_enumerate(this->vendorId, this->productId),
            ::hid_free_enumeration
        );

        auto matchedDevice = std::optional<::hid_device_info*>();

        auto* hidDeviceInfo = hidDeviceInfoList.get();
        while (hidDeviceInfo != nullptr) {
            if (hidDeviceInfo->interface_number == this->interfaceNumber) {
                matchedDevice = hidDeviceInfo;
                break;
            }

            hidDeviceInfo = hidDeviceInfoList->next;
        }

        if (!matchedDevice.has_value()) {
            throw DeviceInitializationFailure("Failed to match interface number with HID interface.");
        }

        return std::string(matchedDevice.value()->path);
    }
}
