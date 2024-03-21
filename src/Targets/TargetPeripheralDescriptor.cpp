#include "TargetPeripheralDescriptor.hpp"

#include <ranges>

#include "src/Services/StringService.hpp"

#include "src/Exceptions/InternalFatalErrorException.hpp"

namespace Targets
{
    TargetPeripheralDescriptor::TargetPeripheralDescriptor(
        const std::string& key,
        const std::string& name,
        const std::map<std::string, TargetRegisterGroupDescriptor, std::less<void>>& registerGroupDescriptorsByKey
    )
        : key(key)
        , name(name)
        , registerGroupDescriptorsByKey(registerGroupDescriptorsByKey)
    {}

    std::optional<
        std::reference_wrapper<const TargetRegisterGroupDescriptor>
    > TargetPeripheralDescriptor::tryGetRegisterGroupDescriptor(std::string_view keyStr) const {
        const auto keys = Services::StringService::split(keyStr, '.');

        const auto firstGroupIt = this->registerGroupDescriptorsByKey.find(*keys.begin());
        return firstGroupIt != this->registerGroupDescriptorsByKey.end()
            ? keys.size() > 1
                ? firstGroupIt->second.tryGetSubgroupDescriptor(keys | std::ranges::views::drop(1))
                : std::optional(std::cref(firstGroupIt->second))
            : std::nullopt;
    }

    const TargetRegisterGroupDescriptor& TargetPeripheralDescriptor::getRegisterGroupDescriptor(
        std::string_view key
    ) const {
        const auto descriptor = this->tryGetRegisterGroupDescriptor(key);
        if (!descriptor.has_value()) {
            throw Exceptions::InternalFatalErrorException(
                "Failed to get register group descriptor \"" + std::string(key)
                    + "\" from peripheral \"" + this->key + "\" - register group descriptor not found"
            );
        }

        return descriptor->get();
    }
}
