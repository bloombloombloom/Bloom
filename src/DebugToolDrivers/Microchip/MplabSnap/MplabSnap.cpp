#include "MplabSnap.hpp"

#include "src/TargetController/Exceptions/DeviceFailure.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

namespace Bloom::DebugToolDrivers
{
    using namespace Protocols::CmsisDap::Edbg::Avr;
    using namespace Bloom::Exceptions;

    using Protocols::CmsisDap::Edbg::EdbgInterface;

    MplabSnap::MplabSnap()
        : UsbDevice(MplabSnap::USB_VENDOR_ID, MplabSnap::USB_PRODUCT_ID)
    {}

    void MplabSnap::init() {
        UsbDevice::init();

        // TODO: Move away from hard-coding the CMSIS-DAP/EDBG interface number
        auto usbHidInterface = Usb::HidInterface(0, this->vendorId, this->productId);

        this->detachKernelDriverFromInterface(usbHidInterface.interfaceNumber);
        usbHidInterface.init();

        this->edbgInterface = std::make_unique<EdbgInterface>(std::move(usbHidInterface));

        this->edbgInterface->setMinimumCommandTimeGap(std::chrono::milliseconds(35));

        // We don't need to claim the CMSISDAP interface here as the HIDAPI will have already done so.
        if (!this->sessionStarted) {
            this->startSession();
        }

        this->edbgAvr8Interface = std::make_unique<EdbgAvr8Interface>(this->edbgInterface.get());
        this->edbgAvrIspInterface = std::make_unique<EdbgAvrIspInterface>(this->edbgInterface.get());

        this->setInitialised(true);
    }

    void MplabSnap::close() {
        if (this->sessionStarted) {
            this->endSession();
        }

        this->edbgInterface->getUsbHidInterface().close();
        UsbDevice::close();
    }

    std::string MplabSnap::getSerialNumber() {
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

    void MplabSnap::startSession() {
        using namespace CommandFrames::HouseKeeping;
        using ResponseFrames::HouseKeeping::ResponseId;

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            StartSession()
        );

        if (responseFrame.id == ResponseId::FAILED) {
            // Failed response returned!
            throw DeviceInitializationFailure("Failed to start session with MPLAB Snap!");
        }

        this->sessionStarted = true;
    }

    void MplabSnap::endSession() {
        using namespace CommandFrames::HouseKeeping;
        using ResponseFrames::HouseKeeping::ResponseId;

        const auto responseFrame = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            EndSession()
        );

        if (responseFrame.id == ResponseId::FAILED) {
            // Failed response returned!
            throw DeviceFailure("Failed to end session with MPLAB Snap!");
        }

        this->sessionStarted = false;
    }
}
