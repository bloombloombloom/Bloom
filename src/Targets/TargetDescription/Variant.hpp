#pragma once

#include <string>
#include <map>
#include <functional>

#include "PropertyGroup.hpp"

#include "src/Services/StringService.hpp"
#include "Exceptions/InvalidTargetDescriptionDataException.hpp"

namespace Targets::TargetDescription
{
    struct Variant
    {
        std::string key;
        std::string name;
        std::string pinoutKey;

        std::map<std::string, PropertyGroup, std::less<void>> propertyGroupsByKey;

        Variant(
            const std::string& key,
            const std::string& name,
            const std::string& pinoutKey,
            const std::map<std::string, PropertyGroup, std::less<void>>& propertyGroupsByKey
        )
            : key(key)
            , name(name)
            , pinoutKey(pinoutKey)
            , propertyGroupsByKey(propertyGroupsByKey)
        {}

        std::optional<std::reference_wrapper<const PropertyGroup>> tryGetPropertyGroup(std::string_view keyStr) const {
            const auto keys = Services::StringService::split(keyStr, '.');

            const auto firstSubgroupIt = this->propertyGroupsByKey.find(*keys.begin());
            return firstSubgroupIt != this->propertyGroupsByKey.end()
                ? keys.size() > 1
                    ? firstSubgroupIt->second.tryGetSubgroup(keys | std::ranges::views::drop(1))
                    : std::optional{std::cref(firstSubgroupIt->second)}
                : std::nullopt;
        }

        const PropertyGroup& getPropertyGroup(std::string_view keyStr) const {
            const auto propertyGroup = this->tryGetPropertyGroup(keyStr);

            if (!propertyGroup.has_value()) {
                throw Exceptions::InvalidTargetDescriptionDataException{
                    "Failed to get property group \"" + std::string{keyStr} + "\" from TDF - property group not found"
                };
            }

            return propertyGroup->get();
        }

        std::optional<std::reference_wrapper<const Property>> tryGetProperty(
            std::string_view groupKey,
            std::string_view propertyKey
        ) const {
            const auto propertyGroup = this->tryGetPropertyGroup(groupKey);

            if (!propertyGroup.has_value()) {
                return std::nullopt;
            }

            return propertyGroup->get().tryGetProperty(propertyKey);
        }

        const Property& getProperty(std::string_view groupKey, std::string_view propertyKey) const {
            const auto property = this->tryGetProperty(groupKey, propertyKey);

            if (!property.has_value()) {
                throw Exceptions::InvalidTargetDescriptionDataException{
                    "Failed to get property \"" + std::string{propertyKey} + "\" from group \"" + std::string{groupKey}
                        + "\", from TDF - property/group not found"
                };
            }

            return property->get();
        }
    };
}
