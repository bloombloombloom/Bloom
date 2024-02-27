#pragma once

#include <cstdint>
#include <string>
#include <optional>

namespace Targets::TargetDescription
{
    struct RegisterGroupInstance
    {
        std::string key;
        std::string name;
        std::string registerGroupKey;
        std::string addressSpaceKey;
        std::uint32_t offset;
        std::optional<std::string> description;

        RegisterGroupInstance(
            const std::string& key,
            const std::string& name,
            const std::string& registerGroupKey,
            const std::string& addressSpaceKey,
            std::uint32_t offset,
            const std::optional<std::string>& description
        )
            : key(key)
            , name(name)
            , registerGroupKey(registerGroupKey)
            , addressSpaceKey(addressSpaceKey)
            , offset(offset)
            , description(description)
        {}
    };
}
