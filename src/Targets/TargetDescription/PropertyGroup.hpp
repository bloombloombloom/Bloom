#pragma once

#include <string>
#include <map>
#include <optional>
#include <string_view>
#include <ranges>
#include <concepts>
#include <functional>

#include "src/Services/StringService.hpp"
#include "Exceptions/InvalidTargetDescriptionDataException.hpp"

namespace Targets::TargetDescription
{
    struct Property
    {
        std::string key;
        std::string value;

        Property(const std::string& key, const std::string& value)
            : key(key)
            , value(value)
        {}
    };

    struct PropertyGroup
    {
        std::string key;
        std::map<std::string, Property, std::less<void>> propertiesByKey;
        std::map<std::string, PropertyGroup, std::less<void>> subgroupsByKey;

        PropertyGroup(
            const std::string& key,
            const std::map<std::string, Property, std::less<void>>& propertiesByKey,
            const std::map<std::string, PropertyGroup, std::less<void>>& subgroupByKey
        )
            : key(key)
            , propertiesByKey(propertiesByKey)
            , subgroupsByKey(subgroupByKey)
        {}

        template <typename KeysType>
        requires
            std::ranges::sized_range<KeysType>
        std::optional<std::reference_wrapper<const PropertyGroup>> tryGetSubgroup(KeysType keys) const {
            auto firstSubgroupIt = this->subgroupsByKey.find(*(keys.begin()));
            if (firstSubgroupIt == this->subgroupsByKey.end()) {
                return std::nullopt;
            }

            auto subgroup = std::optional(std::cref(firstSubgroupIt->second));
            for (const auto key : keys | std::ranges::views::drop(1)) {
                subgroup = subgroup->get().tryGetSubgroup(key);

                if (!subgroup.has_value()) {
                    break;
                }
            }

            return subgroup;
        }

        std::optional<std::reference_wrapper<const PropertyGroup>> tryGetSubgroup(std::string_view keyStr) const {
            return this->tryGetSubgroup(Services::StringService::split(keyStr, '.'));
        }

        const PropertyGroup& getSubgroup(std::string_view keyStr) const {
            const auto propertyGroup = this->tryGetSubgroup(keyStr);
            if (!propertyGroup.has_value()) {
                throw Exceptions::InvalidTargetDescriptionDataException(
                    "Failed to get subgroup \"" + std::string(keyStr)
                        + "\" from property group in TDF - subgroup not found"
                );
            }

            return propertyGroup->get();
        }

        std::optional<std::reference_wrapper<const Property>> tryGetProperty(std::string_view key) const {
            const auto propertyIt = this->propertiesByKey.find(key);

            if (propertyIt == this->propertiesByKey.end()) {
                return std::nullopt;
            }

            return std::cref(propertyIt->second);
        }

        const Property& getProperty(std::string_view key) const {
            const auto property = this->tryGetProperty(key);
            if (!property.has_value()) {
                throw Exceptions::InvalidTargetDescriptionDataException(
                    "Failed to get property \"" + std::string(key) + "\" from property group in TDF - property not found"
                );
            }

            return property->get();
        }
    };
}
