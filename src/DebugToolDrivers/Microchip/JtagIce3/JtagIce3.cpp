#include "JtagIce3.hpp"

namespace Bloom::DebugToolDrivers
{
    JtagIce3::JtagIce3()
        : EdbgDevice(
            JtagIce3::USB_VENDOR_ID,
            JtagIce3::USB_PRODUCT_ID,
            JtagIce3::CMSIS_HID_INTERFACE_NUMBER,
            false,
            JtagIce3::USB_CONFIGURATION_INDEX
        )
    {}
}
