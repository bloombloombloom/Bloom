#pragma once

#include <string>

namespace Targets::TargetDescription
{
    struct Variant
    {
        std::string name;
        std::string pinoutKey;
        std::string package;

        Variant(
            const std::string& name,
            const std::string& pinoutKey,
            const std::string& package
        )
            : name(name)
            , pinoutKey(pinoutKey)
            , package(package)
        {}
    };
}
