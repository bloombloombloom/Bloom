#include "MplabPickit4.hpp"

namespace Bloom::DebugToolDrivers
{
    MplabPickit4::MplabPickit4()
        : EdbgDevice(
            MplabPickit4::USB_VENDOR_ID,
            MplabPickit4::USB_PRODUCT_ID,
            MplabPickit4::CMSIS_HID_INTERFACE_NUMBER
        )
    {}
}
