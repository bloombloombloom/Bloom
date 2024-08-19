#pragma once

#include <string>

namespace Targets::TargetDescription
{
    struct Variant
    {
        std::string key;
        std::string name;
        std::string pinoutKey;

        Variant(
            const std::string& key,
            const std::string& name,
            const std::string& pinoutKey
        )
            : key(key)
            , name(name)
            , pinoutKey(pinoutKey)
        {}
    };
}
