#pragma once

#include <string>
#include <map>
#include <vector>
#include <optional>
#include <functional>
#include <string_view>

#include "TargetRegisterGroupDescriptor.hpp"
#include "TargetRegisterDescriptor.hpp"
#include "TargetPeripheralSignalDescriptor.hpp"

namespace Targets
{
    struct TargetPeripheralDescriptor
    {
    public:
        std::string key;
        std::string name;
        std::map<std::string, TargetRegisterGroupDescriptor, std::less<void>> registerGroupDescriptorsByKey;
        std::vector<TargetPeripheralSignalDescriptor> signalDescriptors;

        TargetPeripheralDescriptor(
            const std::string& key,
            const std::string& name,
            std::map<std::string, TargetRegisterGroupDescriptor, std::less<void>>&& registerGroupDescriptorsByKey,
            std::vector<TargetPeripheralSignalDescriptor>&& signalDescriptors
        );

        TargetPeripheralDescriptor(const TargetPeripheralDescriptor& other) = delete;
        TargetPeripheralDescriptor& operator = (const TargetPeripheralDescriptor& other) = delete;

        TargetPeripheralDescriptor(TargetPeripheralDescriptor&& other) noexcept = default;
        TargetPeripheralDescriptor& operator = (TargetPeripheralDescriptor&& other) = default;

        [[nodiscard]] std::optional<
            std::reference_wrapper<const TargetRegisterGroupDescriptor>
        > tryGetRegisterGroupDescriptor(std::string_view keyStr) const;

        [[nodiscard]] const TargetRegisterGroupDescriptor& getRegisterGroupDescriptor(std::string_view key) const;

        [[nodiscard]] const TargetRegisterDescriptor& getRegisterDescriptor(
            std::string_view groupKey,
            const std::string& registerKey
        ) const;

        [[nodiscard]] TargetPeripheralDescriptor clone() const;
    };
}
