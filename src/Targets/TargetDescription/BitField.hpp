#pragma once

#include <cstdint>
#include <string>

namespace Bloom::Targets::TargetDescription
{
    struct BitField
    {
        std::string name;
        std::uint8_t mask;
    };
}
