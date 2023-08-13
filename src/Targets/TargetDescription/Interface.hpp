#pragma once

#include <string>
#include <optional>

namespace Targets::TargetDescription
{
    struct Interface
    {
        std::string name;
        std::optional<std::string> type;
    };
}
