#pragma once

#include <string>
#include <map>
#include <optional>
#include <string_view>
#include <ranges>
#include <concepts>
#include <functional>

#include "src/Services/StringService.hpp"
#include "src/Exceptions/Exception.hpp"

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
        std::map<std::string, Property, std::less<void>> propertiesMappedByKey;
        std::map<std::string, PropertyGroup, std::less<void>> subGroupsMappedByKey;

        PropertyGroup(
            const std::string& key,
            const std::map<std::string, Property, std::less<void>>& propertiesMappedByKey,
            const std::map<std::string, PropertyGroup, std::less<void>>& subGroupsMappedByKey
        )
            : key(key)
            , propertiesMappedByKey(propertiesMappedByKey)
            , subGroupsMappedByKey(subGroupsMappedByKey)
        {}

        std::optional<std::reference_wrapper<const PropertyGroup>> getSubGroup(std::string_view keyStr) const {
            return this->getSubGroup(Services::StringService::split(keyStr, '.'));
        }

        template <typename KeysType>
        requires
            std::ranges::sized_range<KeysType>
        std::optional<std::reference_wrapper<const PropertyGroup>> getSubGroup(KeysType keys) const {
            auto firstSubGroupIt = this->subGroupsMappedByKey.find(*(keys.begin()));
            if (firstSubGroupIt == this->subGroupsMappedByKey.end()) {
                return std::nullopt;
            }

            auto subGroup = std::optional(std::cref(firstSubGroupIt->second));
            for (const auto key : keys | std::ranges::views::drop(1)) {
                subGroup = subGroup->get().getSubGroup(key);

                if (!subGroup.has_value()) {
                    break;
                }
            }

            return subGroup;
        }

        std::optional<std::reference_wrapper<const Property>> tryGetProperty(std::string_view key) const {
            const auto propertyIt = this->propertiesMappedByKey.find(key);

            if (propertyIt == this->propertiesMappedByKey.end()) {
                return std::nullopt;
            }

            return std::cref(propertyIt->second);
        }

        const Property& getProperty(std::string_view key) const {
            const auto property = this->tryGetProperty(key);
            if (!property.has_value()) {
                throw Exceptions::Exception(
                    "Failed to get property \"" + std::string(key) + "\" from property group in TDF - property not found"
                );
            }

            return property->get();
        }
    };
}
