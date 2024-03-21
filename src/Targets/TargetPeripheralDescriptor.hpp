#pragma once

#include <string>
#include <map>
#include <optional>
#include <functional>
#include <string_view>

#include "TargetRegisterGroupDescriptor.hpp"

namespace Targets
{
    struct TargetPeripheralDescriptor
    {
    public:
        std::string key;
        std::string name;
        std::map<std::string, TargetRegisterGroupDescriptor, std::less<void>> registerGroupDescriptorsByKey;

        TargetPeripheralDescriptor(
            const std::string& key,
            const std::string& name,
            const std::map<std::string, TargetRegisterGroupDescriptor, std::less<void>>& registerGroupDescriptorsByKey
        );

        std::optional<std::reference_wrapper<const TargetRegisterGroupDescriptor>> tryGetRegisterGroupDescriptor(
            std::string_view keyStr
        ) const;

        const TargetRegisterGroupDescriptor& getRegisterGroupDescriptor(std::string_view key) const;
    };
}
