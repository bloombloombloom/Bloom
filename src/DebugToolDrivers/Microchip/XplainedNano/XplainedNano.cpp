#include "XplainedNano.hpp"

namespace DebugToolDrivers::Microchip
{
    XplainedNano::XplainedNano(const DebugToolConfig& debugToolConfig)
        : EdbgDevice(
            debugToolConfig,
            XplainedNano::USB_VENDOR_ID,
            XplainedNano::USB_PRODUCT_ID,
            XplainedNano::CMSIS_HID_INTERFACE_NUMBER,
            true
        )
    {}
}
