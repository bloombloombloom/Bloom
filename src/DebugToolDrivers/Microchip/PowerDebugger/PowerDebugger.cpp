#include "PowerDebugger.hpp"

namespace DebugToolDrivers::Microchip
{
    PowerDebugger::PowerDebugger(const DebugToolConfig& debugToolConfig)
        : EdbgDevice(
            debugToolConfig,
            PowerDebugger::USB_VENDOR_ID,
            PowerDebugger::USB_PRODUCT_ID,
            PowerDebugger::CMSIS_HID_INTERFACE_NUMBER,
            false,
            PowerDebugger::USB_CONFIGURATION_INDEX
        )
    {}
}
