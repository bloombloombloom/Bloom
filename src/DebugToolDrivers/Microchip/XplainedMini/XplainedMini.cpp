#include "XplainedMini.hpp"

namespace DebugToolDrivers::Microchip
{
    XplainedMini::XplainedMini(const DebugToolConfig& debugToolConfig)
        : EdbgDevice(
            debugToolConfig,
            XplainedMini::USB_VENDOR_ID,
            XplainedMini::USB_PRODUCT_ID,
            XplainedMini::CMSIS_HID_INTERFACE_NUMBER,
            true
        )
    {}
}
