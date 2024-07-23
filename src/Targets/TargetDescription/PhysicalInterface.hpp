#pragma once

#include <string>

namespace Targets::TargetDescription
{
    struct PhysicalInterface
    {
        std::string value;

        PhysicalInterface(const std::string& value)
            : value(value)
        {}
    };
}
