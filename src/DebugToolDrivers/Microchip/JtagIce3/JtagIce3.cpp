#include "JtagIce3.hpp"

namespace DebugToolDrivers::Microchip
{
    JtagIce3::JtagIce3(const DebugToolConfig& debugToolConfig)
        : EdbgDevice(
            debugToolConfig,
            JtagIce3::USB_VENDOR_ID,
            JtagIce3::USB_PRODUCT_ID,
            JtagIce3::CMSIS_HID_INTERFACE_NUMBER,
            false,
            JtagIce3::USB_CONFIGURATION_INDEX
        )
    {}
}
