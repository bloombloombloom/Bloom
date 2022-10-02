#include "MplabSnap.hpp"

namespace Bloom::DebugToolDrivers
{
    MplabSnap::MplabSnap()
        : EdbgDevice(
            MplabSnap::USB_VENDOR_ID,
            MplabSnap::USB_PRODUCT_ID,
            MplabSnap::CMSIS_HID_INTERFACE_NUMBER
        )
    {}
}
