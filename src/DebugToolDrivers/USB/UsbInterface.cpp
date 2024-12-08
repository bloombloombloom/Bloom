#include "UsbInterface.hpp"

#include <cassert>
#include <limits>

#include "src/Logger/Logger.hpp"

#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"
#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace Usb
{
    using namespace Exceptions;

    UsbInterface::UsbInterface(
        std::uint8_t interfaceNumber,
        ::libusb_device_handle* deviceHandle
    )
        : interfaceNumber(interfaceNumber)
        , deviceHandle(deviceHandle)
    {}

    UsbInterface::~UsbInterface() {
        this->close();
    }

    void UsbInterface::init() {
        Logger::debug("Claiming USB interface (number: " + std::to_string(this->interfaceNumber) + ")");

        const auto statusCode = ::libusb_claim_interface(this->deviceHandle, this->interfaceNumber);
        if (statusCode != 0) {
            throw DeviceInitializationFailure{
                "Failed to claim USB interface (number: " + std::to_string(this->interfaceNumber) + ") - error code: "
                    + std::to_string(statusCode)
            };
        }

        this->claimed = true;
    }

    void UsbInterface::close() {
        if (this->claimed) {
            const auto statusCode = ::libusb_release_interface(this->deviceHandle, this->interfaceNumber);
            if (statusCode != 0 && statusCode != ::LIBUSB_ERROR_NO_DEVICE) {
                Logger::error(
                    "Failed to release USB interface (number: " + std::to_string(this->interfaceNumber)
                        + ") - error code: " + std::to_string(statusCode)
                );
            }

            this->claimed = false;
        }
    }

    std::vector<unsigned char> UsbInterface::readBulk(
        std::uint8_t endpointAddress,
        std::optional<std::chrono::milliseconds> timeout
    ) {
        auto output = std::vector<unsigned char>{};

        constexpr auto transferSize = 512;
        auto bytesTransferred = int{0};
        auto totalByteCount = std::size_t{0};

        do {
            output.resize(totalByteCount + transferSize, 0x00);

            const auto statusCode = ::libusb_bulk_transfer(
                this->deviceHandle,
                endpointAddress,
                output.data() + totalByteCount,
                transferSize,
                &bytesTransferred,
                timeout.has_value() ? static_cast<unsigned int>(timeout->count()) : 0
            );

            if (statusCode != 0) {
                throw DeviceCommunicationFailure{"Failed to read from bulk endpoint"};
            }

            if (totalByteCount == 0) {
                // After the first read, set the timeout to 1 millisecond, as we don't want to wait for subsequent data
                timeout = std::chrono::milliseconds{1};
            }

            totalByteCount += static_cast<std::size_t>(bytesTransferred);

        } while (bytesTransferred >= transferSize);

        output.resize(totalByteCount, 0x00);
        return output;
    }

    void UsbInterface::writeBulk(
        std::uint8_t endpointAddress,
        std::span<const unsigned char> buffer,
        std::uint16_t maxPacketSize
    ) {
        assert(buffer.size() <= std::numeric_limits<int>::max());

        const auto bufferSize = buffer.size();
        auto totalBytesTransferred = std::size_t{0};

        while (totalBytesTransferred < bufferSize) {
            auto bytesTransferred = int{0};
            const auto length = std::min(
                static_cast<int>(bufferSize - totalBytesTransferred),
                static_cast<int>(maxPacketSize)
            );

            const auto statusCode = ::libusb_bulk_transfer(
                this->deviceHandle,
                endpointAddress,
                /*
                 * I know, I know...this is horrible. I'm effectively lying about the constness of `buffer`. But libusb
                 * *shouldn't* write to `buffer` here, because we're not reading from the device.
                 */
                const_cast<unsigned char*>(buffer.data()) + totalBytesTransferred,
                length,
                &bytesTransferred,
                0
            );

            if (statusCode != 0 || bytesTransferred != length) {
                Logger::debug(
                    "Attempted to write " + std::to_string(length) + " bytes to USB bulk endpoint. Bytes written: "
                        + std::to_string(bytesTransferred) + ". Status code: " + std::to_string(statusCode)
                );
                throw DeviceCommunicationFailure{"Failed to write data to bulk endpoint"};
            }

            totalBytesTransferred += static_cast<std::size_t>(bytesTransferred);
        }
    }
}
