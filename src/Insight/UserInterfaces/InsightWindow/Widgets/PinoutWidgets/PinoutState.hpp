#pragma once

#include <cstdint>
#include <optional>

namespace Widgets::PinoutWidgets
{
    struct PinoutState
    {
        std::optional<std::uint16_t> hoveredPinNumber = std::nullopt;
    };
}
