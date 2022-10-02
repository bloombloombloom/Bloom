#include "XplainedMini.hpp"

namespace Bloom::DebugToolDrivers
{
    XplainedMini::XplainedMini()
        : EdbgDevice(
            XplainedMini::USB_VENDOR_ID,
            XplainedMini::USB_PRODUCT_ID,
            XplainedMini::CMSIS_HID_INTERFACE_NUMBER,
            true
        )
    {}
}
