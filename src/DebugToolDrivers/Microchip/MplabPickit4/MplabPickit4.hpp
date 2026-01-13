#pragma once

#include <cstdint>
#include <string>

#include "src/DebugToolDrivers/Microchip/EdbgDevice.hpp"

namespace DebugToolDrivers::Microchip
{
    class MplabPickit4: public EdbgDevice
    {
    public:
        static constexpr std::uint16_t USB_VENDOR_ID = 0x03eb;
        static constexpr std::uint16_t USB_PRODUCT_ID = 0x2177;
        static constexpr std::uint8_t CMSIS_HID_INTERFACE_NUMBER = 0;

        static constexpr std::uint16_t PIC_MODE_USB_VENDOR_ID = 0x04d8;
        static constexpr std::uint16_t PIC_MODE_USB_PRODUCT_ID = 0x9012;
        static constexpr std::uint16_t BL_MODE_USB_PRODUCT_ID = 0x9017;

        explicit MplabPickit4(const DebugToolConfig& debugToolConfig);

        std::string getName() override {
            return "MPLAB PICkit 4";
        }

        void init() override;
    };
}
