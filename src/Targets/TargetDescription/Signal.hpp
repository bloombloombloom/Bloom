#pragma once

#include <string>
#include <optional>

namespace Targets::TargetDescription
{
    struct Signal
    {
        std::string padName;
        std::string function;
        std::optional<int> index;
        std::string group;
    };
}
