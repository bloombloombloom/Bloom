#pragma once

#include <cstdint>

namespace TargetController
{
    enum class TargetControllerState: std::uint8_t
    {
        ACTIVE,
        INACTIVE,
    };
}
