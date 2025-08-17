#pragma once

#include <cstdint>
#include <string>

#include "src/DebugToolDrivers/Microchip/EdbgDevice.hpp"

namespace DebugToolDrivers::Microchip
{
    class AtmelIce: public EdbgDevice
    {
    public:
        static constexpr std::uint16_t USB_VENDOR_ID = 0x03eb;
        static constexpr std::uint16_t USB_PRODUCT_ID = 0x2141;
        static constexpr std::uint8_t USB_CONFIGURATION_INDEX = 0;
        static constexpr std::uint8_t CMSIS_HID_INTERFACE_NUMBER = 0;

        explicit AtmelIce(const DebugToolConfig& debugToolConfig);

        std::string getName() override {
            return "Atmel-ICE";
        }
    };
}
