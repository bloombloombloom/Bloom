#pragma once

#include <string>

namespace Bloom::Targets::TargetDescription
{
    struct Variant
    {
        std::string name;
        std::string pinoutName;
        std::string package;
        bool disabled = false;
    };
}
