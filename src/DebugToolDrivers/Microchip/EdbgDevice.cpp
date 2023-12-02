#include "EdbgDevice.hpp"

#include "src/DebugToolDrivers/USB/HID/HidInterface.hpp"
#include "src/DebugToolDrivers/Microchip/Protocols/EDBG/AVR/CommandFrames/AvrCommandFrames.hpp"

#include "src/TargetController/Exceptions/DeviceFailure.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

namespace DebugToolDrivers::Microchip
{
    using namespace Microchip::Protocols::Edbg::Avr;

    using Exceptions::DeviceFailure;
    using Exceptions::DeviceInitializationFailure;

    EdbgDevice::EdbgDevice(
        std::uint16_t vendorId,
        std::uint16_t productId,
        std::uint8_t cmsisHidInterfaceNumber,
        bool supportsTargetPowerManagement,
        std::optional<std::uint8_t> configurationIndex
    )
        : UsbDevice(vendorId, productId)
        , cmsisHidInterfaceNumber(cmsisHidInterfaceNumber)
        , supportsTargetPowerManagement(supportsTargetPowerManagement)
        , configurationIndex(configurationIndex)
    {}

    void EdbgDevice::init() {
        using Microchip::Protocols::Edbg::EdbgInterface;
        using Microchip::Protocols::Edbg::EdbgTargetPowerManagementInterface;

        UsbDevice::init();

        this->detachKernelDriverFromInterface(this->cmsisHidInterfaceNumber);

        if (this->configurationIndex.has_value()) {
            this->setConfiguration(this->configurationIndex.value());
        }

        auto cmsisHidInterface = Usb::HidInterface(
            this->cmsisHidInterfaceNumber,
            this->getEndpointMaxPacketSize(
                this->getFirstEndpointAddress(this->cmsisHidInterfaceNumber, LIBUSB_ENDPOINT_IN)
            ),
            this->vendorId,
            this->productId
        );

        cmsisHidInterface.init();

        this->edbgInterface = std::make_unique<EdbgInterface>(std::move(cmsisHidInterface));

        /*
         * The EDBG/CMSIS-DAP interface doesn't operate properly when sending commands too quickly.
         *
         * Because of this, we have to enforce a minimum time gap between commands. See comment
         * in CmsisDapInterface class declaration for more info.
         */
        this->edbgInterface->setMinimumCommandTimeGap(std::chrono::milliseconds(35));

        // We don't need to claim the CMSISDAP interface here as the HIDAPI will have already done so.
        if (!this->sessionStarted) {
            this->startSession();
        }

        if (this->supportsTargetPowerManagement) {
            this->targetPowerManagementInterface = std::make_unique<EdbgTargetPowerManagementInterface>(
                this->edbgInterface.get()
            );
        }

        this->edbgAvrIspInterface = std::make_unique<EdbgAvrIspInterface>(this->edbgInterface.get());

        this->setInitialised(true);
    }

    void EdbgDevice::close() {
        if (this->sessionStarted) {
            this->endSession();
        }

        this->edbgInterface->getUsbHidInterface().close();
        UsbDevice::close();
    }

    TargetInterfaces::Microchip::Avr::Avr8::Avr8DebugInterface* EdbgDevice::getAvr8DebugInterface(
        const Targets::Microchip::Avr::Avr8Bit::Avr8TargetConfig& targetConfig,
        Targets::Microchip::Avr::Avr8Bit::Family targetFamily,
        const Targets::Microchip::Avr::Avr8Bit::TargetParameters& targetParameters,
        const Targets::TargetRegisterDescriptorMapping& targetRegisterDescriptorsById
    ) {
        if (this->edbgAvr8Interface == nullptr) {
            this->edbgAvr8Interface = std::make_unique<EdbgAvr8Interface>(
                this->edbgInterface.get(),
                targetConfig,
                targetFamily,
                targetParameters,
                targetRegisterDescriptorsById
            );

            this->configureAvr8Interface();
        }

        return this->edbgAvr8Interface.get();
    }

    std::string EdbgDevice::getSerialNumber() {
        using namespace CommandFrames::Discovery;
        using ResponseFrames::Discovery::ResponseId;

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            Query(QueryContext::SERIAL_NUMBER)
        );

        if (responseFrame.id != ResponseId::OK) {
            throw DeviceInitializationFailure(
                "Failed to fetch serial number from device - invalid Discovery Protocol response ID."
            );
        }

        const auto data = responseFrame.getPayloadData();
        return std::string(data.begin(), data.end());
    }

    std::string EdbgDevice::getFirmwareVersionString() {
        // TODO: Implement this
        return "UNKNOWN";
    }

    void EdbgDevice::startSession() {
        using namespace CommandFrames::HouseKeeping;
        using ResponseFrames::HouseKeeping::ResponseId;

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            StartSession()
        );

        if (responseFrame.id == ResponseId::FAILED) {
            // Failed response returned!
            throw DeviceInitializationFailure("Failed to start session with EDBG device!");
        }

        this->sessionStarted = true;
    }

    void EdbgDevice::endSession() {
        using namespace CommandFrames::HouseKeeping;
        using ResponseFrames::HouseKeeping::ResponseId;

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            EndSession()
        );

        if (responseFrame.id == ResponseId::FAILED) {
            // Failed response returned!
            throw DeviceFailure("Failed to end session with EDBG device!");
        }

        this->sessionStarted = false;
    }
}
