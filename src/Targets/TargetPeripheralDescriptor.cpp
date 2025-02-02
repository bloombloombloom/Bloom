#include "TargetPeripheralDescriptor.hpp"

#include <ranges>
#include <utility>

#include "src/Services/StringService.hpp"

#include "src/Exceptions/InternalFatalErrorException.hpp"

namespace Targets
{
    TargetPeripheralDescriptor::TargetPeripheralDescriptor(
        const std::string& key,
        const std::string& name,
        const std::string& description,
        std::map<std::string, TargetRegisterGroupDescriptor, std::less<void>>&& registerGroupDescriptorsByKey,
        std::vector<TargetPeripheralSignalDescriptor>&& signalDescriptors
    )
        : id(static_cast<TargetPeripheralId>(Services::StringService::generateUniqueInteger(key)))
        , key(key)
        , name(name)
        , description(description)
        , registerGroupDescriptorsByKey(std::move(registerGroupDescriptorsByKey))
        , signalDescriptors(std::move(signalDescriptors))
    {}

    bool TargetPeripheralDescriptor::operator == (const TargetPeripheralDescriptor& other) const {
        return this->id == other.id;
    }

    bool TargetPeripheralDescriptor::operator != (const TargetPeripheralDescriptor& other) const {
        return !(*this == other);
    }

    std::optional<
        std::reference_wrapper<const TargetRegisterGroupDescriptor>
    > TargetPeripheralDescriptor::tryGetRegisterGroupDescriptor(std::string_view keyStr) const {
        const auto keys = Services::StringService::split(keyStr, '.');

        const auto firstGroupIt = this->registerGroupDescriptorsByKey.find(*keys.begin());
        return firstGroupIt != this->registerGroupDescriptorsByKey.end()
            ? keys.size() > 1
                ? firstGroupIt->second.tryGetSubgroupDescriptor(keys | std::ranges::views::drop(1))
                : std::optional{std::cref(firstGroupIt->second)}
            : std::nullopt;
    }

    const TargetRegisterGroupDescriptor& TargetPeripheralDescriptor::getRegisterGroupDescriptor(
        std::string_view key
    ) const {
        const auto descriptor = this->tryGetRegisterGroupDescriptor(key);
        if (!descriptor.has_value()) {
            throw Exceptions::InternalFatalErrorException{
                "Failed to get register group descriptor \"" + std::string{key}
                    + "\" from peripheral `" + this->key + "` - register group descriptor not found"
            };
        }

        return descriptor->get();
    }

    const TargetRegisterDescriptor& TargetPeripheralDescriptor::getRegisterDescriptor(
        std::string_view groupKey,
        const std::string& registerKey
    ) const {
        return this->getRegisterGroupDescriptor(groupKey).getRegisterDescriptor(registerKey);
    }

    std::optional<
        std::reference_wrapper<const TargetPeripheralSignalDescriptor>
    > TargetPeripheralDescriptor::tryGetFirstSignalDescriptor(std::string_view padKey) const {
        for (const auto& signalDescriptor : this->signalDescriptors) {
            if (signalDescriptor.padKey == padKey) {
                return std::cref(signalDescriptor);
            }
        }

        return std::nullopt;
    }

    TargetPeripheralDescriptor TargetPeripheralDescriptor::clone() const {
        auto output = TargetPeripheralDescriptor{
            this->key,
            this->name,
            this->description,
            {},
            {}
        };

        for (const auto& [key, descriptor] : this->registerGroupDescriptorsByKey) {
            output.registerGroupDescriptorsByKey.emplace(key, descriptor.clone());
        }

        for (const auto& descriptor : this->signalDescriptors) {
            output.signalDescriptors.emplace_back(descriptor.clone());
        }

        return output;
    }
}
