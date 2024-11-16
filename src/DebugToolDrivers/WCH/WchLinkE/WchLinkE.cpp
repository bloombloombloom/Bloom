#include "WchLinkE.hpp"

namespace DebugToolDrivers::Wch
{
    WchLinkE::WchLinkE(const DebugToolConfig& toolConfig)
        : WchLinkBase(
            toolConfig,
            WchLinkVariant::LINK_E_CH32V307,
            WchLinkE::USB_VENDOR_ID,
            WchLinkE::USB_PRODUCT_ID,
            WchLinkE::WCH_LINK_INTERFACE_NUMBER
        )
    {}

    std::string WchLinkE::getName() {
        return "WCH-LinkE";
    }
}
