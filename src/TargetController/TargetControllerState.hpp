#pragma once

#include <cstdint>

namespace Bloom::TargetController
{
    enum class TargetControllerState: std::uint8_t
    {
        ACTIVE,
        SUSPENDED,
    };
}
