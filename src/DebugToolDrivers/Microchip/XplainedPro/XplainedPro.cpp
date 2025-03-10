#include "XplainedPro.hpp"

namespace DebugToolDrivers::Microchip
{
    XplainedPro::XplainedPro(const DebugToolConfig& debugToolConfig)
        : EdbgDevice(
            debugToolConfig,
            XplainedPro::USB_VENDOR_ID,
            XplainedPro::USB_PRODUCT_ID,
            XplainedPro::CMSIS_HID_INTERFACE_NUMBER,
            true
        )
    {}

    void XplainedPro::configureAvr8Interface() {
        this->edbgAvr8Interface->setMaximumMemoryAccessSizePerRequest(256);
    }
}
