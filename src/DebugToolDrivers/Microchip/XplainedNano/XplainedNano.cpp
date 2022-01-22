#include "XplainedNano.hpp"

#include "src/TargetController/Exceptions/DeviceFailure.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

using namespace Bloom::DebugToolDrivers;
using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr;
using namespace Bloom::Exceptions;

void XplainedNano::init() {
    UsbDevice::init();

    // TODO: Move away from hard-coding the CMSIS-DAP/EDBG interface number
    auto& usbHidInterface = this->getEdbgInterface().getUsbHidInterface();
    usbHidInterface.setNumber(0);
    usbHidInterface.setLibUsbDevice(this->libUsbDevice);
    usbHidInterface.setLibUsbDeviceHandle(this->libUsbDeviceHandle);
    usbHidInterface.setVendorId(this->vendorId);
    usbHidInterface.setProductId(this->productId);

    if (!usbHidInterface.isInitialised()) {
        usbHidInterface.detachKernelDriver();
        usbHidInterface.init();
    }

    this->getEdbgInterface().setMinimumCommandTimeGap(std::chrono::milliseconds(35));

    if (!this->sessionStarted) {
        this->startSession();
    }

    this->edbgAvr8Interface = std::make_unique<EdbgAvr8Interface>(this->edbgInterface);
    this->setInitialised(true);
}

void XplainedNano::close() {
    if (this->sessionStarted) {
        this->endSession();
    }

    this->getEdbgInterface().getUsbHidInterface().close();
    UsbDevice::close();
}

std::string XplainedNano::getSerialNumber() {
    auto response = this->getEdbgInterface().sendAvrCommandFrameAndWaitForResponseFrame(
        CommandFrames::Discovery::Query(CommandFrames::Discovery::QueryContext::SERIAL_NUMBER)
    );

    if (response.getResponseId() != CommandFrames::Discovery::ResponseId::OK) {
        throw DeviceInitializationFailure(
            "Failed to fetch serial number from device - invalid Discovery Protocol response ID."
        );
    }

    auto data = response.getPayloadData();
    return std::string(data.begin(), data.end());
}

void XplainedNano::startSession() {
    auto response = this->getEdbgInterface().sendAvrCommandFrameAndWaitForResponseFrame(
        CommandFrames::HouseKeeping::StartSession()
    );

    if (response.getResponseId() == CommandFrames::HouseKeeping::ResponseId::FAILED) {
        // Failed response returned!
        throw DeviceInitializationFailure("Failed to start session with Xplained Nano!");
    }

    this->sessionStarted = true;
}

void XplainedNano::endSession() {
    auto response = this->getEdbgInterface().sendAvrCommandFrameAndWaitForResponseFrame(
        CommandFrames::HouseKeeping::EndSession()
    );

    if (response.getResponseId() == CommandFrames::HouseKeeping::ResponseId::FAILED) {
        // Failed response returned!
        throw DeviceFailure("Failed to end session with Xplained Nano!");
    }

    this->sessionStarted = false;
}
