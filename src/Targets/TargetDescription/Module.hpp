#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <string_view>
#include <optional>
#include <functional>

#include "ModuleInstance.hpp"
#include "RegisterGroup.hpp"

#include "src/Services/StringService.hpp"

namespace Targets::TargetDescription
{
    struct Module
    {
        std::string key;
        std::string name;
        std::string description;
        std::map<std::string, RegisterGroup, std::less<void>> registerGroupsByKey;

        Module(
            const std::string& key,
            const std::string& name,
            const std::string& description,
            const std::map<std::string, RegisterGroup, std::less<void>>& registerGroupsByKey
        )
            : key(key)
            , name(name)
            , description(description)
            , registerGroupsByKey(registerGroupsByKey)
        {}

        std::optional<std::reference_wrapper<const RegisterGroup>> tryGetRegisterGroup(std::string_view keyStr) const {
            const auto keys = Services::StringService::split(keyStr, '.');

            const auto firstGroupIt = this->registerGroupsByKey.find(*keys.begin());
            return firstGroupIt != this->registerGroupsByKey.end()
                ? keys.size() > 1
                    ? firstGroupIt->second.tryGetSubgroup(keys | std::ranges::views::drop(1))
                    : std::optional(std::cref(firstGroupIt->second))
                : std::nullopt;
        }

        std::optional<std::reference_wrapper<const RegisterGroup>> getRegisterGroup(std::string_view keyStr) const {
            const auto group = this->tryGetRegisterGroup(keyStr);
            if (!group.has_value()) {
                throw Exceptions::InvalidTargetDescriptionDataException(
                    "Failed to get register group \"" + std::string(keyStr)
                        + "\" from module in TDF - register group not found"
                );
            }

            return group->get();
        }
    };
}
