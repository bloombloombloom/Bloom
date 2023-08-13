#pragma once

#include <cstdint>
#include <string>

#include "src/DebugToolDrivers/Microchip/EdbgDevice.hpp"

namespace DebugToolDrivers
{
    /**
     * The MPLAB Snap device is a hybrid device - that is, it can present itself as an "MPLAB Snap ICD" device, as well
     * as an EDBG (Embedded Debugger) device. The device switches between these two modes via firmware configuration.
     * It appears that it can only interface with AVR targets using the EDBG firmware, which, apparently, is why it is
     * known to be in "AVR mode" when it presents itself as an EDBG device.
     *
     * This debug tool driver currently only supports the device when in AVR mode. Because the device uses different
     * USB vendor and product IDs depending on the mode, it is trivial to determine which is which. In fact, Bloom will
     * not even recognise the device if it's not in AVR mode.
     *
     * USB (when in AVR/EDBG mode):
     *  Vendor ID: 0x03eb (1003)
     *  Product ID: 0x2180 (8576)
     */
    class MplabSnap: public EdbgDevice
    {
    public:
        static const inline std::uint16_t USB_VENDOR_ID = 0x03eb;
        static const inline std::uint16_t USB_PRODUCT_ID = 0x2180;
        static const inline std::uint8_t CMSIS_HID_INTERFACE_NUMBER = 0;

        static const inline std::uint16_t NON_EDBG_USB_VENDOR_ID = 0x04d8;
        static const inline std::uint16_t NON_EDBG_USB_PRODUCT_ID = 0x9018;
        static const inline std::uint16_t NON_EDBG_USB_PRODUCT_ID_ALTERNATIVE = 0x9017;

        MplabSnap();

        std::string getName() override {
            return "MPLAB Snap";
        }

        void init() override;

    protected:
        void configureAvr8Interface() override;
    };
}
