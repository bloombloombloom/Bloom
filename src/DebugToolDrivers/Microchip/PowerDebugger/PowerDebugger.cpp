#include "PowerDebugger.hpp"

namespace Bloom::DebugToolDrivers
{
    PowerDebugger::PowerDebugger()
        : EdbgDevice(
            PowerDebugger::USB_VENDOR_ID,
            PowerDebugger::USB_PRODUCT_ID,
            PowerDebugger::CMSIS_HID_INTERFACE_NUMBER,
            false,
            PowerDebugger::USB_CONFIGURATION_INDEX
        )
    {}
}
