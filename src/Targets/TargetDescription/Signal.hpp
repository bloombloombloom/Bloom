#pragma once

#include <cstdint>
#include <string>
#include <optional>

namespace Targets::TargetDescription
{
    struct Signal
    {
        std::string name;
        std::string padKey;
        std::optional<bool> alternative;
        std::optional<std::uint16_t> index;
        std::optional<std::string> function;
        std::optional<std::string> field;

        Signal(
            const std::string& name,
            const std::string& padKey,
            const std::optional<bool>& alternative,
            const std::optional<std::uint16_t>& index,
            const std::optional<std::string>& function,
            const std::optional<std::string>& field
        )
            : name(name)
            , padKey(padKey)
            , alternative(alternative)
            , index(index)
            , function(function)
            , field(field)
        {}
    };
}
