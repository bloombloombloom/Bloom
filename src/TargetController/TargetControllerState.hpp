#pragma once

#include <cstdint>

namespace Bloom
{
    enum class TargetControllerState: std::uint8_t
    {
        ACTIVE,
        SUSPENDED,
    };
}
