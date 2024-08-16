#pragma once

#include <string>
#include <optional>

namespace Targets::TargetDescription
{
    struct Pin
    {
        std::string position;
        std::optional<std::string> padKey;

        Pin(
            const std::string& position,
            const std::optional<std::string>& padKey
        )
            : position(position)
            , padKey(padKey)
        {}
    };
}
