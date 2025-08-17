#pragma once

#include <cstdint>
#include <string>

#include "src/DebugToolDrivers/Microchip/EdbgDevice.hpp"

namespace DebugToolDrivers::Microchip
{
    class XplainedMini: public EdbgDevice
    {
    public:
        static constexpr std::uint16_t USB_VENDOR_ID = 0x03eb;
        static constexpr std::uint16_t USB_PRODUCT_ID = 0x2145;
        static constexpr std::uint8_t CMSIS_HID_INTERFACE_NUMBER = 0;

        explicit XplainedMini(const DebugToolConfig& debugToolConfig);

        std::string getName() override {
            return "Xplained Mini";
        }
    };
}
