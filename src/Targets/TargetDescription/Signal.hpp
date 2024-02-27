#pragma once

#include <cstdint>
#include <string>
#include <optional>

namespace Targets::TargetDescription
{
    struct Signal
    {
        std::string padId;
        std::optional<std::uint16_t> index;
        std::optional<std::string> function;
        std::optional<std::string> group;
        std::optional<std::string> field;

        Signal(
            const std::string& padId,
            const std::optional<std::uint16_t>& index,
            const std::optional<std::string>& function,
            const std::optional<std::string>& group,
            const std::optional<std::string>& field
        )
            : padId(padId)
            , index(index)
            , function(function)
            , group(group)
            , field(field)
        {}
    };
}
