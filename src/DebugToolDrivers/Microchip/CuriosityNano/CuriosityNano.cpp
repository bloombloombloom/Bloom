#include "CuriosityNano.hpp"

namespace DebugToolDrivers::Microchip
{
    CuriosityNano::CuriosityNano(const DebugToolConfig& debugToolConfig)
        : EdbgDevice(
            debugToolConfig,
            CuriosityNano::USB_VENDOR_ID,
            CuriosityNano::USB_PRODUCT_ID,
            CuriosityNano::CMSIS_HID_INTERFACE_NUMBER,
            true
        )
    {}
}
