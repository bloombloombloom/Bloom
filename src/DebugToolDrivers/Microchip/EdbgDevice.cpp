#include "EdbgDevice.hpp"

#include "src/DebugToolDrivers/USB/HID/HidInterface.hpp"
#include "src/DebugToolDrivers/Protocols/CMSIS-DAP/VendorSpecific/EDBG/AVR/CommandFrames/AvrCommandFrames.hpp"

#include "src/TargetController/Exceptions/DeviceFailure.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

namespace Bloom::DebugToolDrivers
{
    using namespace Protocols::CmsisDap::Edbg::Avr;
    using namespace Bloom::Exceptions;

    using Protocols::CmsisDap::Edbg::EdbgInterface;
    using Protocols::CmsisDap::Edbg::EdbgTargetPowerManagementInterface;

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
        UsbDevice::init();

        this->detachKernelDriverFromInterface(this->cmsisHidInterfaceNumber);

        if (this->configurationIndex.has_value()) {
            this->setConfiguration(this->configurationIndex.value());
        }

        auto cmsisHidInterface = Usb::HidInterface(
            this->cmsisHidInterfaceNumber,
            this->getCmsisHidReportSize(),
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

        this->edbgAvr8Interface = std::make_unique<EdbgAvr8Interface>(this->edbgInterface.get());
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

    std::uint16_t EdbgDevice::getCmsisHidReportSize() {
        const auto activeConfigDescriptor = this->getConfigDescriptor();

        for (auto interfaceIndex = 0; interfaceIndex < activeConfigDescriptor->bNumInterfaces; ++interfaceIndex) {
            const auto* interfaceDescriptor = (activeConfigDescriptor->interface + interfaceIndex)->altsetting;

            if (interfaceDescriptor->bInterfaceNumber != this->cmsisHidInterfaceNumber) {
                continue;
            }

            for (auto endpointIndex = 0; endpointIndex < activeConfigDescriptor->bNumInterfaces; ++endpointIndex) {
                const auto* endpointDescriptor = (interfaceDescriptor->endpoint + endpointIndex);

                if ((endpointDescriptor->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) != LIBUSB_ENDPOINT_IN) {
                    // Not an IN endpoint
                    continue;
                }

                return endpointDescriptor->wMaxPacketSize;
            }
        }

        throw DeviceInitializationFailure(
            "Failed to obtain CMSIS-DAP HID report size via endpoint descriptor - could not find IN endpoint for "
            "selected configuration value (" + std::to_string(activeConfigDescriptor->bConfigurationValue)
            + ") and interface number (" + std::to_string(this->cmsisHidInterfaceNumber) + ")"
        );
    }
}
