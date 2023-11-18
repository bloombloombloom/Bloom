#pragma once

#include <cstdint>

namespace DebugToolDrivers::Wch
{
    enum class WchLinkVariant: std::uint8_t
    {
        LINK_CH549,
        LINK_E_CH32V307,
        LINK_S_CH32V203,
        UNKNOWN,
    };
}
