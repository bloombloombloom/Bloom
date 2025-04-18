#pragma once

#include <cstdint>
#include <string>
#include <optional>

namespace Targets::TargetDescription
{
    struct BitField
    {
        std::string key;
        std::string name;
        std::optional<std::string> description;
        std::uint64_t mask;
        std::optional<std::string> access;

        BitField(
            const std::string& key,
            const std::string& name,
            const std::optional<std::string>& description,
            std::uint64_t mask,
            const std::optional<std::string>& access
        )
            : key(key)
            , name(name)
            , description(description)
            , mask(mask)
            , access(access)
        {}
    };
}
