#pragma once

#include <cstdint>

namespace Targets
{
    enum class TargetFamily: std::uint8_t
    {
        AVR_8,
        RISC_V,
    };
}
