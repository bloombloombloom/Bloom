#pragma once

#include <cstdint>
#include <string>

namespace Targets::TargetDescription
{
    struct PhysicalInterface
    {
        std::string name;
        std::string type;

        PhysicalInterface(
            const std::string& name,
            const std::string& type
        )
            : name(name)
            , type(type)
        {}
    };
}
