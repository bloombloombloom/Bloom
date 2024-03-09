#pragma once

#include <string>

namespace Targets::TargetDescription
{
    struct Pin
    {
        std::string position;
        std::string pad;

        Pin(
            const std::string& position,
            const std::string& pad
        )
            : position(position)
            , pad(pad)
        {}
    };
}
