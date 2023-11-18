#include "AtmelIce.hpp"

namespace DebugToolDrivers::Microchip
{
    AtmelIce::AtmelIce()
        : EdbgDevice(
            AtmelIce::USB_VENDOR_ID,
            AtmelIce::USB_PRODUCT_ID,
            AtmelIce::CMSIS_HID_INTERFACE_NUMBER,
            false,
            AtmelIce::USB_CONFIGURATION_INDEX
        )
    {}
}
