#pragma once

#include <cstdint>
#include <string>

#include "src/DebugToolDrivers/Microchip/EdbgDevice.hpp"

namespace DebugToolDrivers::Microchip
{
    /**
     * The Atmel-ICE device is an EDBG (Embedded Debugger) device.
     *
     * USB:
     *  Vendor ID: 0x03eb (1003)
     *  Product ID: 0x2141 (8513)
     */
    class AtmelIce: public EdbgDevice
    {
    public:
        static const inline std::uint16_t USB_VENDOR_ID = 0x03eb;
        static const inline std::uint16_t USB_PRODUCT_ID = 0x2141;
        static const inline std::uint8_t USB_CONFIGURATION_INDEX = 0;
        static const inline std::uint8_t CMSIS_HID_INTERFACE_NUMBER = 0;

        AtmelIce();

        std::string getName() override {
            return "Atmel-ICE";
        }
    };
}
