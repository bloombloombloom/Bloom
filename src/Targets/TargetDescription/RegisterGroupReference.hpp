#pragma once

#include <cstdint>
#include <string>
#include <optional>

namespace Targets::TargetDescription
{
    struct RegisterGroupReference
    {
        std::string key;
        std::string name;
        std::string registerGroupKey;
        std::uint32_t offset;
        std::optional<std::string> description;

        RegisterGroupReference(
            const std::string& key,
            const std::string& name,
            const std::string& registerGroupKey,
            std::uint32_t offset,
            const std::optional<std::string>& description
        )
            : key(key)
            , name(name)
            , registerGroupKey(registerGroupKey)
            , offset(offset)
            , description(description)
        {}
    };
}
