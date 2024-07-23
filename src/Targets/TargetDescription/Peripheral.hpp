#pragma once

#include <cstdint>
#include <string>
#include <map>
#include <functional>
#include <vector>

#include "RegisterGroupInstance.hpp"
#include "Signal.hpp"

#include "Exceptions/InvalidTargetDescriptionDataException.hpp"

namespace Targets::TargetDescription
{
    struct Peripheral
    {
        std::string key;
        std::string name;
        std::string moduleKey;

        std::map<std::string, RegisterGroupInstance, std::less<void>> registerGroupInstancesByKey;
        std::vector<Signal> sigs;

        Peripheral(
            const std::string& key,
            const std::string& name,
            const std::string& moduleKey,
            const std::map<std::string, RegisterGroupInstance, std::less<void>>& registerGroupInstancesByKey,
            const std::vector<Signal>& sigs
        )
            : key(key)
            , name(name)
            , moduleKey(moduleKey)
            , registerGroupInstancesByKey(registerGroupInstancesByKey)
            , sigs(sigs)
        {}

        std::optional<std::reference_wrapper<const RegisterGroupInstance>> tryGetRegisterGroupInstance(
            std::string_view key
        ) const {
            const auto instanceIt = this->registerGroupInstancesByKey.find(key);

            if (instanceIt == this->registerGroupInstancesByKey.end()) {
                return std::nullopt;
            }

            return std::cref(instanceIt->second);
        }

        const RegisterGroupInstance& getRegisterGroupInstance(std::string_view key) const {
            const auto instance = this->tryGetRegisterGroupInstance(key);
            if (!instance.has_value()) {
                throw Exceptions::InvalidTargetDescriptionDataException{
                    "Failed to get register group instance \"" + std::string{key}
                        + "\" from peripheral in TDF - register group instance not found"
                };
            }

            return instance->get();
        }
    };
}
