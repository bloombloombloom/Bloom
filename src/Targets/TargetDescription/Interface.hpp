#pragma once

#include <string>
#include <optional>

namespace Bloom::Targets::TargetDescription
{
    struct Interface
    {
        std::string name;
        std::optional<std::string> type;
    };
}
