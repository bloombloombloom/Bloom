#include "WchLinkInterface.hpp"

#include <cassert>

#include "Commands/Control/GetDeviceInfo.hpp"
#include "Commands/Control/AttachTarget.hpp"
#include "Commands/SetClockSpeed.hpp"
#include "Commands/DebugModuleInterfaceOperation.hpp"

#include "src/Helpers/BiMap.hpp"
#include "src/Services/StringService.hpp"

#include "src/Logger/Logger.hpp"

#include "src/TargetController/Exceptions/DeviceCommunicationFailure.hpp"

namespace DebugToolDrivers::Wch::Protocols::WchLink
{
    using namespace Exceptions;

    using Targets::RiscV::DebugModule::DmiOperation;

    WchLinkInterface::WchLinkInterface(Usb::UsbInterface& usbInterface)
        : usbInterface(usbInterface)
    {}

    DeviceInfo WchLinkInterface::getDeviceInfo() {
        const auto response = this->sendCommandAndWaitForResponse(Commands::Control::GetDeviceInfo());

        if (response.payload.size() < 3) {
            throw Exceptions::DeviceCommunicationFailure("Cannot construct DeviceInfo response - invalid payload");
        }

        static const auto variantsById = BiMap<std::uint8_t, WchLinkVariant>({
            {0x01, WchLinkVariant::LINK_CH549},
            {0x02, WchLinkVariant::LINK_E_CH32V307},
            {0x12, WchLinkVariant::LINK_E_CH32V307},
            {0x03, WchLinkVariant::LINK_S_CH32V203},
        });

        return DeviceInfo(
            WchFirmwareVersion(response.payload[0], response.payload[1]),
            response.payload.size() >= 4
                ? std::optional(variantsById.valueAt(response.payload[2]).value_or(WchLinkVariant::UNKNOWN))
                : std::nullopt
        );
    }

    void WchLinkInterface::activate(const Targets::RiscV::TargetParameters& targetParameters) {
        this->setClockSpeed(WchLinkTargetClockSpeed::CLK_6000_KHZ);

        const auto response = this->sendCommandAndWaitForResponse(Commands::Control::AttachTarget());

        if (response.payload.size() != 5) {
            throw Exceptions::DeviceCommunicationFailure("Unexpected response payload size for AttachTarget command");
        }

        this->cachedTargetId = static_cast<WchTargetId>(
            (response.payload[1] << 24) | (response.payload[2] << 16) | (response.payload[3] << 8)
                | (response.payload[4])
        );
        this->cachedTargetGroupId = response.payload[0];
    }

    void WchLinkInterface::deactivate() {
        // TODO: implement this
    }

    std::string WchLinkInterface::getDeviceId() {
        return "0x" + Services::StringService::toHex(this->cachedTargetId.value());
    }

    Targets::RiscV::DebugModule::RegisterValue WchLinkInterface::readDebugModuleRegister(
        Targets::RiscV::DebugModule::RegisterAddress address
    ) {
        using Targets::RiscV::DebugModule::DmiOperationStatus ;

        const auto response = this->sendCommandAndWaitForResponse(
            Commands::DebugModuleInterfaceOperation(DmiOperation::READ, address)
        );

        if (response.operationStatus != DmiOperationStatus::SUCCESS) {
            throw Exceptions::DeviceCommunicationFailure("DMI operation failed");
        }

        return response.value;
    }

    void WchLinkInterface::writeDebugModuleRegister(
        Targets::RiscV::DebugModule::RegisterAddress address,
        Targets::RiscV::DebugModule::RegisterValue value
    ) {
        using Targets::RiscV::DebugModule::DmiOperationStatus ;

        const auto response = this->sendCommandAndWaitForResponse(
            Commands::DebugModuleInterfaceOperation(DmiOperation::WRITE, address, value)
        );

        if (response.operationStatus != DmiOperationStatus::SUCCESS) {
            throw Exceptions::DeviceCommunicationFailure("DMI operation failed");
        }
    }

    void WchLinkInterface::setClockSpeed(WchLinkTargetClockSpeed speed) {
        const auto speedIdsBySpeed = BiMap<WchLinkTargetClockSpeed, std::uint8_t>({
            {WchLinkTargetClockSpeed::CLK_6000_KHZ, 0x01},
            {WchLinkTargetClockSpeed::CLK_4000_KHZ, 0x02},
            {WchLinkTargetClockSpeed::CLK_400_KHZ, 0x03},
        });

        const auto response = this->sendCommandAndWaitForResponse(
            Commands::SetClockSpeed(this->cachedTargetGroupId.value_or(0x01), speedIdsBySpeed.at(speed))
        );

        if (response.payload.size() != 1) {
            throw Exceptions::DeviceCommunicationFailure("Unexpected response payload size for SetClockSpeed command");
        }

        if (response.payload[0] != 0x01) {
            throw Exceptions::DeviceCommunicationFailure("Unexpected response for SetClockSpeed command");
        }
    }
}
