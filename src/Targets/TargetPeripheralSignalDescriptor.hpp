#pragma once

#include <cstdint>
#include <string>
#include <optional>

#include "TargetPadDescriptor.hpp"

namespace Targets
{
    struct TargetPeripheralSignalDescriptor
    {
    public:
        const TargetPadId padId;
        const std::string padKey;
        std::optional<std::uint16_t> index;

        TargetPeripheralSignalDescriptor(
            const std::string& padKey,
            const std::optional<std::uint16_t>& index
        );

        TargetPeripheralSignalDescriptor(const TargetPeripheralSignalDescriptor& other) = delete;
        TargetPeripheralSignalDescriptor& operator = (const TargetPeripheralSignalDescriptor& other) = delete;

        TargetPeripheralSignalDescriptor(TargetPeripheralSignalDescriptor&& other) noexcept = default;

        [[nodiscard]] TargetPeripheralSignalDescriptor clone() const;
    };
}
