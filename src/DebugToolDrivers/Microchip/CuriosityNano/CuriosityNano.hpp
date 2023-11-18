#pragma once

#include <cstdint>
#include <string>

#include "src/DebugToolDrivers/Microchip/EdbgDevice.hpp"

namespace DebugToolDrivers::Microchip
{
    /**
     * The Curiosity Nano is an evaluation board featuring an on-board debugger. The debugger is EDBG-based.
     *
     * USB:
     *  Vendor ID: 0x03eb (1003)
     *  Product ID: 0x2175 (8565)
     */
    class CuriosityNano: public EdbgDevice
    {
    public:
        static const inline std::uint16_t USB_VENDOR_ID = 0x03eb;
        static const inline std::uint16_t USB_PRODUCT_ID = 0x2175;
        static const inline std::uint8_t CMSIS_HID_INTERFACE_NUMBER = 0;

        CuriosityNano();

        std::string getName() override {
            return "Curiosity Nano";
        }
    };
}
