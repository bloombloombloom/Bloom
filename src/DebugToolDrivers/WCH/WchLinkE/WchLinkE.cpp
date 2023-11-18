#include "WchLinkE.hpp"

namespace DebugToolDrivers::Wch
{
    WchLinkE::WchLinkE()
        : WchLinkBase(
            WchLinkVariant::LINK_E_CH32V307,
            WchLinkE::USB_VENDOR_ID,
            WchLinkE::USB_PRODUCT_ID,
            WchLinkE::WCH_LINK_INTERFACE_NUMBER
        )
    {}
}
