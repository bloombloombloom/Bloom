#include "XplainedPro.hpp"

namespace Bloom::DebugToolDrivers
{
    XplainedPro::XplainedPro()
        : EdbgDevice(
            XplainedPro::USB_VENDOR_ID,
            XplainedPro::USB_PRODUCT_ID,
            XplainedPro::CMSIS_HID_INTERFACE_NUMBER,
            true
        )
    {}
}
