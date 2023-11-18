#pragma once

#include <cstdint>
#include <string>

#include "src/DebugToolDrivers/WCH/WchLinkBase.hpp"

namespace DebugToolDrivers::Wch
{
    /**
     * The WCH-LinkE debug tool is a variant of the WCH-Link.
     *
     * USB:
     *  Vendor ID: 0x1a86 (6790)
     *  Product ID: 0x8010 (32784)
     */
    class WchLinkE: public WchLinkBase
    {
    public:
        static const inline std::uint16_t USB_VENDOR_ID = 0x1a86;
        static const inline std::uint16_t USB_PRODUCT_ID = 0x8010;
        static const inline std::uint8_t WCH_LINK_INTERFACE_NUMBER = 0;

        WchLinkE();

        std::string getName() override {
            return "WCH-LinkE";
        }
    };
}
