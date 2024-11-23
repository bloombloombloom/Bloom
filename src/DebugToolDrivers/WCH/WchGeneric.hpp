#pragma once

#include <cstdint>

namespace DebugToolDrivers::Wch
{
    using WchTargetId = std::uint8_t;
    using WchTargetVariantId = std::uint32_t;

    enum class WchLinkVariant: std::uint8_t
    {
        LINK_CH549,
        LINK_E_CH32V307,
        LINK_S_CH32V203,
    };

    enum class WchLinkTargetClockSpeed: std::uint8_t
    {
        CLK_400_KHZ,
        CLK_4000_KHZ,
        CLK_6000_KHZ,
    };
}
