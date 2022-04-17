#pragma once

#include <cstdint>

namespace Bloom::TargetController::Responses
{
    enum class ResponseType: std::uint8_t
    {
        GENERIC,
        ERROR,
    };
}
