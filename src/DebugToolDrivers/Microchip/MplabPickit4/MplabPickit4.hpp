#pragma once

#include <cstdint>
#include <string>

#include "src/DebugToolDrivers/Microchip/EdbgDevice.hpp"

namespace Bloom::DebugToolDrivers
{
    /**
     * Like the MPLAB Snap, the PICkit 4 is a hybrid device. It can present itself as an EDBG (Embedded Debugger)
     * device via firmware configuration, actioned by Microchip software (the MPLAB IDE and IPE).
     *
     * This debug tool driver currently only supports the device when in AVR mode. Because the device uses different
     * USB vendor and product IDs depending on the mode, it is trivial to determine which is which. In fact, Bloom will
     * not even recognise the device if it's not in AVR mode.
     *
     * USB (when in AVR/EDBG mode):
     *  Vendor ID: 0x03eb (1003)
     *  Product ID: 0x2177 (8567)
     */
    class MplabPickit4: public EdbgDevice
    {
    public:
        static const inline std::uint16_t USB_VENDOR_ID = 0x03eb;
        static const inline std::uint16_t USB_PRODUCT_ID = 0x2177;
        static const inline std::uint8_t CMSIS_HID_INTERFACE_NUMBER = 0;

        static const inline std::uint16_t NON_EDBG_USB_VENDOR_ID = 0x04d8;
        static const inline std::uint16_t NON_EDBG_USB_PRODUCT_ID = 0x9012;

        MplabPickit4();

        std::string getName() override {
            return "MPLAB PICkit 4";
        }

        void init() override;
    };
}
