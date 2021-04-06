#include "PowerDebugger.hpp"
#include "src/Exceptions/Exception.hpp"

using namespace Bloom::DebugToolDrivers;
using namespace Bloom::DebugToolDrivers::Protocols::CmsisDap::Edbg::Avr;
using namespace Bloom::Exceptions;

void PowerDebugger::init() {
    UsbDevice::init();

    // TODO: Move away from hard-coding the CMSIS-DAP/EDBG interface number
    auto& usbHidInterface = this->getEdbgInterface().getUsbHidInterface();
    usbHidInterface.setNumber(0);
    usbHidInterface.setUSBDevice(this->getLibUsbDevice());
    usbHidInterface.setVendorId(this->getVendorId());
    usbHidInterface.setProductId(this->getProductId());

    if (!usbHidInterface.isInitialised()) {
        usbHidInterface.init();
    }

    /**
     * The Power Debugger EDBG/CMSIS-DAP interface doesn't operate properly when sending commands too quickly.
     *
     * Because of this, we have to enforce a minimum time gap between commands. See comment in
     * CmsisDapInterface class declaration for more info.
     */
    this->getEdbgInterface().setMinimumCommandTimeGap(35);

    // We don't need to claim the CMSISDAP interface here as the HIDAPI will have already done so.
    if (!this->sessionStarted) {
        this->startSession();
    }

    this->edbgAvr8Interface = std::make_unique<EdbgAvr8Interface>(this->edbgInterface);
    this->setInitialised(true);
}

void PowerDebugger::close() {
    if (this->sessionStarted) {
        this->endSession();
    }

    this->getEdbgInterface().getUsbHidInterface().close();
    UsbDevice::close();
}

std::string PowerDebugger::getSerialNumber() {
    auto response = this->getEdbgInterface().sendAvrCommandFrameAndWaitForResponseFrame(
        CommandFrames::Discovery::Query(Discovery::QueryContext::SERIAL_NUMBER)
    );

    if (response.getResponseId() != Discovery::ResponseId::OK) {
        throw Exception("Failed to fetch serial number from device - invalid Discovery Protocol response ID.");
    }

    auto data = response.getPayloadData();
    return std::string(data.begin(), data.end());
}

void PowerDebugger::startSession() {
    auto response = this->getEdbgInterface().sendAvrCommandFrameAndWaitForResponseFrame(
        CommandFrames::HouseKeeping::StartSession()
    );

    if (response.getResponseId() == HouseKeeping::ResponseId::FAILED) {
        // Failed response returned!
        throw Exception("Failed to start session with the Power Debugger - device returned failed response ID");
    }

    this->sessionStarted = true;
}

void PowerDebugger::endSession() {
    auto response = this->getEdbgInterface().sendAvrCommandFrameAndWaitForResponseFrame(
        CommandFrames::HouseKeeping::EndSession()
    );

    if (response.getResponseId() == HouseKeeping::ResponseId::FAILED) {
        // Failed response returned!
        throw Exception("Failed to end session with the Power Debugger - device returned failed response ID");
    }

    this->sessionStarted = false;
}
