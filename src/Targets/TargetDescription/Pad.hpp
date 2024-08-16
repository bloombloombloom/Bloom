#pragma once

#include <string>

namespace Targets::TargetDescription
{
    struct Pad
    {
        std::string key;
        std::string name;

        Pad(
            const std::string& key,
            const std::string& name
        )
            : key(key)
            , name(name)
        {}
    };
}
