#include "WchLinkInterface.hpp"

#include <cassert>
#include <thread>

#include "Commands/Control/GetDeviceInfo.hpp"
#include "Commands/Control/AttachTarget.hpp"
#include "Commands/Control/DetachTarget.hpp"
#include "Commands/Control/PostAttach.hpp"
#include "Commands/SetClockSpeed.hpp"
#include "Commands/DebugModuleInterfaceOperation.hpp"
#include "Commands/PreparePartialFlashPageWrite.hpp"
#include "Commands/StartProgrammingSession.hpp"
#include "Commands/EndProgrammingSession.hpp"
#include "Commands/SetFlashWriteRegion.hpp"
#include "Commands/StartRamCodeWrite.hpp"
#include "Commands/EndRamCodeWrite.hpp"
#include "Commands/WriteFlash.hpp"
#include "Commands/EraseChip.hpp"

#include "src/Helpers/BiMap.hpp"
#include "src/Services/StringService.hpp"

#include "src/Logger/Logger.hpp"

#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink
{
    using namespace ::DebugToolDrivers::Protocols::RiscVDebugSpec;
    using namespace Exceptions;

    using DebugModule::DmiOperation;

    WchLinkInterface::WchLinkInterface(Usb::UsbInterface& usbInterface, Usb::UsbDevice& usbDevice)
        : usbInterface(usbInterface)
        , commandEndpointMaxPacketSize(usbDevice.getEndpointMaxPacketSize(WchLinkInterface::USB_COMMAND_ENDPOINT_OUT))
        , dataEndpointMaxPacketSize(usbDevice.getEndpointMaxPacketSize(WchLinkInterface::USB_DATA_ENDPOINT_OUT))
    {}

    DeviceInfo WchLinkInterface::getDeviceInfo() {
        const auto response = this->sendCommandAndWaitForResponse(Commands::Control::GetDeviceInfo{});
        if (response.payload.size() < 3) {
            throw Exceptions::DeviceCommunicationFailure{"Cannot construct DeviceInfo response - invalid payload"};
        }

        static const auto variantsById = BiMap<std::uint8_t, WchLinkVariant>{
            {0x01, WchLinkVariant::LINK_CH549},
            {0x02, WchLinkVariant::LINK_E_CH32V307},
            {0x12, WchLinkVariant::LINK_E_CH32V307},
            {0x03, WchLinkVariant::LINK_S_CH32V203},
        };

        return DeviceInfo{
            WchFirmwareVersion{response.payload[0], response.payload[1]},
            response.payload.size() >= 4
                ? variantsById.valueAt(response.payload[2])
                : std::nullopt
        };
    }

    void WchLinkInterface::setClockSpeed(WchLinkTargetClockSpeed speed, WchTargetId targetId) {
        const auto speedIdsBySpeed = BiMap<WchLinkTargetClockSpeed, std::uint8_t>{
            {WchLinkTargetClockSpeed::CLK_6000_KHZ, 0x01},
            {WchLinkTargetClockSpeed::CLK_4000_KHZ, 0x02},
            {WchLinkTargetClockSpeed::CLK_400_KHZ, 0x03},
        };

        const auto response = this->sendCommandAndWaitForResponse(
            Commands::SetClockSpeed{targetId, speedIdsBySpeed.at(speed)}
        );

        if (response.payload.size() != 1) {
            throw Exceptions::DeviceCommunicationFailure{"Unexpected response payload size for SetClockSpeed command"};
        }

        if (response.payload[0] != 0x01) {
            throw Exceptions::DeviceCommunicationFailure{"Unexpected response for SetClockSpeed command"};
        }
    }

    DebugModule::RegisterValue WchLinkInterface::readDebugModuleRegister(DebugModule::RegisterAddress address) {
        using DebugModule::DmiOperationStatus;

        auto attempt = std::uint8_t{0};
        while (attempt < WchLinkInterface::DMI_OP_MAX_RETRY) {
            if (attempt > 0) {
                std::this_thread::sleep_for(this->dmiOpRetryDelay);
            }

            const auto response = this->sendCommandAndWaitForResponse(
                Commands::DebugModuleInterfaceOperation{DmiOperation::READ, address}
            );

            if (response.operationStatus == DmiOperationStatus::SUCCESS) {
                return response.value;
            }

            if (response.operationStatus == DmiOperationStatus::FAILED) {
                throw Exceptions::DeviceCommunicationFailure{"DMI operation failed"};
            }

            // Busy response...
            ++attempt;
        }

        throw Exceptions::DeviceCommunicationFailure{"DMI operation timed out"};
    }

    void WchLinkInterface::writeDebugModuleRegister(
        DebugModule::RegisterAddress address,
        DebugModule::RegisterValue value
    ) {
        using DebugModule::DmiOperationStatus;

        auto attempt = std::uint8_t{0};
        while (attempt < WchLinkInterface::DMI_OP_MAX_RETRY) {
            if (attempt > 0) {
                std::this_thread::sleep_for(this->dmiOpRetryDelay);
            }

            const auto response = this->sendCommandAndWaitForResponse(
                Commands::DebugModuleInterfaceOperation{DmiOperation::WRITE, address, value}
            );

            if (response.operationStatus == DmiOperationStatus::SUCCESS) {
                return;
            }

            if (response.operationStatus == DmiOperationStatus::FAILED) {
                throw Exceptions::DeviceCommunicationFailure{"DMI operation failed"};
            }

            // Busy response...
            ++attempt;
        }

        throw Exceptions::DeviceCommunicationFailure{"DMI operation timed out"};
    }

    void WchLinkInterface::writePartialPage(
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemoryBufferSpan buffer
    ) {
        constexpr auto packetSize = std::uint8_t{64};
        const auto bufferSize = static_cast<Targets::TargetMemorySize>(buffer.size());
        const auto packetsRequired = static_cast<std::uint32_t>(
            std::ceil(static_cast<float>(bufferSize) / static_cast<float>(packetSize))
        );

        for (auto i = std::uint32_t{0}; i < packetsRequired; ++i) {
            const auto segmentSize = static_cast<std::uint8_t>(
                std::min(
                    static_cast<std::uint8_t>(bufferSize - (i * packetSize)),
                    packetSize
                )
            );
            const auto response = this->sendCommandAndWaitForResponse(
                Commands::PreparePartialFlashPageWrite{startAddress + (packetSize * i), segmentSize}
            );

            if (response.payload.size() != 1) {
                throw Exceptions::DeviceCommunicationFailure{
                    "Unexpected response payload size for PreparePartialFlashPageWrite command"
                };
            }

            this->usbInterface.writeBulk(
                WchLinkInterface::USB_DATA_ENDPOINT_OUT,
                buffer.subspan(packetSize * i, segmentSize),
                this->dataEndpointMaxPacketSize
            );

            const auto rawResponse = this->usbInterface.readBulk(WchLinkInterface::USB_DATA_ENDPOINT_IN);
            if (rawResponse.size() != 4) {
                throw Exceptions::DeviceCommunicationFailure{"Unexpected response size for partial flash page write"};
            }

            /*
             * I have no idea what any of these bytes mean. I've not been able to find any documentation for
             * this. All I know is that these values indicate a successful write.
             */
            if (rawResponse[0] != 0x41 || rawResponse[1] != 0x01 || rawResponse[2] != 0x01 || rawResponse[3] != 0x02) {
                throw Exceptions::DeviceCommunicationFailure{"Partial flash page write failed"};
            }
        }
    }

    void WchLinkInterface::writeFullPage(
        Targets::TargetMemoryAddress startAddress,
        Targets::TargetMemoryBufferSpan buffer,
        Targets::TargetMemorySize pageSize,
        std::span<const unsigned char> flashProgramOpcodes
    ) {
        assert((buffer.size() % pageSize) == 0);

        const auto bufferSize = static_cast<Targets::TargetMemorySize>(buffer.size());

        /*
         * We don't issue the StartProgrammingSession command here, as it seems to result in a failure when writing a
         * flash page. We get a 0x05 in rawResponse[3], but I have no idea why.
         */
        this->sendCommandAndWaitForResponse(Commands::SetFlashWriteRegion{startAddress, bufferSize});
        this->sendCommandAndWaitForResponse(Commands::StartRamCodeWrite{});

        this->usbInterface.writeBulk(
            WchLinkInterface::USB_DATA_ENDPOINT_OUT,
            flashProgramOpcodes,
            this->dataEndpointMaxPacketSize
        );

        this->sendCommandAndWaitForResponse(Commands::EndRamCodeWrite{});
        this->sendCommandAndWaitForResponse(Commands::WriteFlash{});

        auto bytesWritten = Targets::TargetMemorySize{0};
        while (bytesWritten < bufferSize) {
            const auto length = std::min(bufferSize - bytesWritten, pageSize);

            this->usbInterface.writeBulk(
                WchLinkInterface::USB_DATA_ENDPOINT_OUT,
                buffer.subspan(bytesWritten, length),
                this->dataEndpointMaxPacketSize
            );

            const auto rawResponse = this->usbInterface.readBulk(WchLinkInterface::USB_DATA_ENDPOINT_IN);
            if (rawResponse.size() != 4) {
                throw Exceptions::DeviceCommunicationFailure{"Unexpected response size for flash page write"};
            }

            if (rawResponse[3] != 0x02 && rawResponse[3] != 0x04) {
                throw Exceptions::DeviceCommunicationFailure{
                    "Flash page write failed - unexpected response (0x"
                        + Services::StringService::toHex(rawResponse[3]) + ")"
                };
            }

            bytesWritten += length;
        }

        this->sendCommandAndWaitForResponse(Commands::EndProgrammingSession{});
    }

    void WchLinkInterface::eraseChip() {
        this->sendCommandAndWaitForResponse(Commands::EraseChip{});
    }
}
