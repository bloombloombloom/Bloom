#pragma once

#include <cstdint>
#include <string>

#include "src/DebugToolDrivers/Microchip/EdbgDevice.hpp"

namespace DebugToolDrivers::Microchip
{
    /**
     * The Xplained Nano is an evaluation board featuring an on-board debugger. The debugger is EDBG-based.
     *
     * USB:
     *  Vendor ID: 0x03eb (1003)
     *  Product ID: 0x2145 (8517)
     */
    class XplainedNano: public EdbgDevice
    {
    public:
        static const inline std::uint16_t USB_VENDOR_ID = 0x03eb;
        static const inline std::uint16_t USB_PRODUCT_ID = 0x2145;
        static const inline std::uint8_t CMSIS_HID_INTERFACE_NUMBER = 0;

        XplainedNano(const DebugToolConfig& debugToolConfig);

        std::string getName() override {
            return "Xplained Nano";
        }
    };
}
