#include "XplainedMini.hpp"

#include "src/TargetController/Exceptions/DeviceFailure.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

using namespace Bloom::DebugToolDrivers;
using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr;
using namespace Bloom::Exceptions;

void XplainedMini::init() {
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
    this->edbgAvr8Interface->setAvoidMaskedMemoryRead(true);
    this->setInitialised(true);
}

void XplainedMini::close() {
    if (this->sessionStarted) {
        this->endSession();
    }

    this->getEdbgInterface().getUsbHidInterface().close();
    UsbDevice::close();
}

std::string XplainedMini::getSerialNumber() {
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

void XplainedMini::startSession() {
    auto response = this->getEdbgInterface().sendAvrCommandFrameAndWaitForResponseFrame(
        CommandFrames::HouseKeeping::StartSession()
    );

    if (response.getResponseId() == CommandFrames::HouseKeeping::ResponseId::FAILED) {
        // Failed response returned!
        throw DeviceInitializationFailure("Failed to start session with Xplained Mini!");
    }

    this->sessionStarted = true;
}

void XplainedMini::endSession() {
    auto response = this->getEdbgInterface().sendAvrCommandFrameAndWaitForResponseFrame(
        CommandFrames::HouseKeeping::EndSession()
    );

    if (response.getResponseId() == CommandFrames::HouseKeeping::ResponseId::FAILED) {
        // Failed response returned!
        throw DeviceFailure("Failed to end session with Xplained Mini!");
    }

    this->sessionStarted = false;
}