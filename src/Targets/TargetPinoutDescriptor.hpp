#pragma once

#include <cstdint>

namespace Targets
{
    enum class PinoutType: std::uint8_t
    {
        SOIC,
        SSOP,
        DIP,
        QFN,
        QFP,
        DUAL_ROW_QFN,
        MLF,
        BGA,
    };
}
