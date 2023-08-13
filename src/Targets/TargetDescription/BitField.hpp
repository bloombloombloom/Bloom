#pragma once

#include <cstdint>
#include <string>

namespace Targets::TargetDescription
{
    struct BitField
    {
        std::string name;
        std::uint8_t mask;
    };
}
