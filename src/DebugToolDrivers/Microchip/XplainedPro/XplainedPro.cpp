#include "XplainedPro.hpp"

#include "src/TargetController/Exceptions/DeviceFailure.hpp"
#include "src/TargetController/Exceptions/DeviceInitializationFailure.hpp"

namespace Bloom::DebugToolDrivers
{
    using namespace Protocols::CmsisDap::Edbg::Avr;
    using namespace Bloom::Exceptions;

    void XplainedPro::init() {
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

        /*
         * The Xplained Pro debug tool returns incorrect data for any read memory command that exceeds 256 bytes in the
         * size of the read request, despite the fact that the HID report size is 512 bytes. The debug tool doesn't
         * report any error, it just returns incorrect data.
         *
         * This means we must enforce a hard limit on the number of bytes we attempt to access, per request.
         */
        this->edbgAvr8Interface->setMaximumMemoryAccessSizePerRequest(256);
        this->setInitialised(true);
    }

    void XplainedPro::close() {
        if (this->sessionStarted) {
            this->endSession();
        }

        this->getEdbgInterface().getUsbHidInterface().close();
        UsbDevice::close();
    }

    std::string XplainedPro::getSerialNumber() {
        using namespace CommandFrames::Discovery;
        using ResponseFrames::Discovery::ResponseId;

        auto response = this->getEdbgInterface().sendAvrCommandFrameAndWaitForResponseFrame(
            Query(QueryContext::SERIAL_NUMBER)
        );

        if (response.getResponseId() != ResponseId::OK) {
            throw DeviceInitializationFailure(
                "Failed to fetch serial number from device - invalid Discovery Protocol response ID."
            );
        }

        auto data = response.getPayloadData();
        return std::string(data.begin(), data.end());
    }

    void XplainedPro::startSession() {
        using namespace CommandFrames::HouseKeeping;

        auto response = this->getEdbgInterface().sendAvrCommandFrameAndWaitForResponseFrame(
            StartSession()
        );

        if (response.getResponseId() == ResponseId::FAILED) {
            // Failed response returned!
            throw DeviceInitializationFailure("Failed to start session with Xplained Pro!");
        }

        this->sessionStarted = true;
    }

    void XplainedPro::endSession() {
        using namespace CommandFrames::HouseKeeping;

        auto response = this->getEdbgInterface().sendAvrCommandFrameAndWaitForResponseFrame(
            EndSession()
        );

        if (response.getResponseId() == ResponseId::FAILED) {
            // Failed response returned!
            throw DeviceFailure("Failed to end session with Xplained Pro!");
        }

        this->sessionStarted = false;
    }
}
