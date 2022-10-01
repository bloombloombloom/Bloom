#include "XplainedNano.hpp"

#include "src/TargetController/Exceptions/DeviceFailure.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

namespace Bloom::DebugToolDrivers
{
    using namespace Protocols::CmsisDap::Edbg::Avr;
    using namespace Bloom::Exceptions;

    using Protocols::CmsisDap::Edbg::EdbgInterface;
    using Protocols::CmsisDap::Edbg::EdbgTargetPowerManagementInterface;

    XplainedNano::XplainedNano()
        : UsbDevice(XplainedNano::USB_VENDOR_ID, XplainedNano::USB_PRODUCT_ID)
    {}

    void XplainedNano::init() {
        UsbDevice::init();

        // TODO: Move away from hard-coding the CMSIS-DAP/EDBG interface number
        auto usbHidInterface = Usb::HidInterface(0, this->vendorId, this->productId);

        this->detachKernelDriverFromInterface(usbHidInterface.interfaceNumber);
        usbHidInterface.init();

        this->edbgInterface = std::make_unique<EdbgInterface>(std::move(usbHidInterface));

        this->edbgInterface->setMinimumCommandTimeGap(std::chrono::milliseconds(35));

        if (!this->sessionStarted) {
            this->startSession();
        }

        this->targetPowerManagementInterface = std::make_unique<EdbgTargetPowerManagementInterface>(
            this->edbgInterface.get()
        );

        this->edbgAvr8Interface = std::make_unique<EdbgAvr8Interface>(this->edbgInterface.get());
        this->setInitialised(true);
    }

    void XplainedNano::close() {
        if (this->sessionStarted) {
            this->endSession();
        }

        this->edbgInterface->getUsbHidInterface().close();
        UsbDevice::close();
    }

    std::string XplainedNano::getSerialNumber() {
        using namespace CommandFrames::Discovery;
        using ResponseFrames::Discovery::ResponseId;

        auto response = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            Query(QueryContext::SERIAL_NUMBER)
        );

        if (response.getResponseId() != ResponseId::OK) {
            throw DeviceInitializationFailure(
                "Failed to fetch serial number from device - invalid Discovery Protocol response ID."
            );
        }

        const auto data = response.getPayloadData();
        return std::string(data.begin(), data.end());
    }

    void XplainedNano::startSession() {
        using namespace CommandFrames::HouseKeeping;
        using ResponseFrames::HouseKeeping::ResponseId;

        auto response = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            StartSession()
        );

        if (response.getResponseId() == ResponseId::FAILED) {
            // Failed response returned!
            throw DeviceInitializationFailure("Failed to start session with Xplained Nano!");
        }

        this->sessionStarted = true;
    }

    void XplainedNano::endSession() {
        using namespace CommandFrames::HouseKeeping;
        using ResponseFrames::HouseKeeping::ResponseId;

        auto response = this->edbgInterface->sendAvrCommandFrameAndWaitForResponseFrame(
            EndSession()
        );

        if (response.getResponseId() == ResponseId::FAILED) {
            // Failed response returned!
            throw DeviceFailure("Failed to end session with Xplained Nano!");
        }

        this->sessionStarted = false;
    }
}
