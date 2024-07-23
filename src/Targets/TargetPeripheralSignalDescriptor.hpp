#pragma once

#include <cstdint>
#include <string>
#include <optional>

namespace Targets
{
    struct TargetPeripheralSignalDescriptor
    {
    public:
        std::string padName;
        std::optional<std::uint16_t> index;

        TargetPeripheralSignalDescriptor(
            const std::string& padName,
            const std::optional<std::uint16_t>& index
        );

        TargetPeripheralSignalDescriptor(const TargetPeripheralSignalDescriptor& other) = delete;
        TargetPeripheralSignalDescriptor& operator = (const TargetPeripheralSignalDescriptor& other) = delete;

        TargetPeripheralSignalDescriptor(TargetPeripheralSignalDescriptor&& other) noexcept = default;
        TargetPeripheralSignalDescriptor& operator = (TargetPeripheralSignalDescriptor&& other) = default;

        [[nodiscard]] TargetPeripheralSignalDescriptor clone() const;
    };
}
