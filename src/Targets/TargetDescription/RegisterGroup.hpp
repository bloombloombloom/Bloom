#pragma once

#include <cstdint>
#include <string>
#include <optional>
#include <map>
#include <string_view>
#include <ranges>
#include <concepts>
#include <functional>

#include "Register.hpp"
#include "RegisterGroupReference.hpp"

namespace Targets::TargetDescription
{
    struct RegisterGroup
    {
        std::string key;
        std::string name;
        std::optional<std::uint32_t> offset;
        std::map<std::string, Register, std::less<void>> registersByKey;
        std::map<std::string, RegisterGroup, std::less<void>> subgroupsByKey;
        std::map<std::string, RegisterGroupReference, std::less<void>> subgroupReferencesByKey;

        RegisterGroup(
            const std::string& key,
            const std::string& name,
            const std::optional<std::uint32_t>& offset,
            const std::map<std::string, Register, std::less<void>>& registersByKey,
            const std::map<std::string, RegisterGroup, std::less<void>>& subgroupsByKey,
            const std::map<std::string, RegisterGroupReference, std::less<void>>& subgroupReferencesByKey
        )
            : key(key)
            , name(name)
            , offset(offset)
            , registersByKey(registersByKey)
            , subgroupsByKey(subgroupsByKey)
            , subgroupReferencesByKey(subgroupReferencesByKey)
        {}

        template <typename KeysType>
        requires
            std::ranges::sized_range<KeysType>
        std::optional<std::reference_wrapper<const RegisterGroup>> tryGetSubgroup(KeysType keys) const {
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

        std::optional<std::reference_wrapper<const RegisterGroup>> tryGetSubgroup(std::string_view keyStr) const {
            return this->tryGetSubgroup(Services::StringService::split(keyStr, '.'));
        }

        const RegisterGroup& getSubgroup(std::string_view keyStr) const {
            const auto subgroup = this->tryGetSubgroup(keyStr);
            if (!subgroup.has_value()) {
                throw Exceptions::InvalidTargetDescriptionDataException(
                    "Failed to get subgroup \"" + std::string(keyStr)
                        + "\" from register group in TDF - subgroup not found"
                );
            }

            return subgroup->get();
        }

        std::optional<std::reference_wrapper<const Register>> tryGetRegister(std::string_view key) const {
            const auto registerIt = this->registersByKey.find(key);

            if (registerIt == this->registersByKey.end()) {
                return std::nullopt;
            }

            return std::cref(registerIt->second);
        }

        const Register& getRegister(std::string_view key) const {
            const auto reg = this->tryGetRegister(key);
            if (!reg.has_value()) {
                throw Exceptions::InvalidTargetDescriptionDataException(
                    "Failed to get register \"" + std::string(key) + "\" from register group in TDF - register "
                        "not found"
                );
            }

            return reg->get();
        }
    };
}
