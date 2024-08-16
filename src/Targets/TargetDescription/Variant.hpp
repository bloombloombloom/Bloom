#pragma once

#include <string>

namespace Targets::TargetDescription
{
    struct Variant
    {
        std::string name;
        std::string pinoutKey;

        Variant(
            const std::string& name,
            const std::string& pinoutKey
        )
            : name(name)
            , pinoutKey(pinoutKey)
        {}
    };
}
