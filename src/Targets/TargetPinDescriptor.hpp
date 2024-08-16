#pragma once

#include <cstdint>
#include <string>
#include <optional>

namespace Targets
{
    struct TargetPinDescriptor
    {
    public:
        std::string position;
        std::uint16_t numericPosition;
        std::optional<std::string> padKey;

        TargetPinDescriptor(
            const std::string& position,
            const std::optional<std::string>& padKey
        );

        TargetPinDescriptor(const TargetPinDescriptor& other) = delete;
        TargetPinDescriptor& operator = (const TargetPinDescriptor& other) = delete;

        TargetPinDescriptor(TargetPinDescriptor&& other) noexcept = default;
        TargetPinDescriptor& operator = (TargetPinDescriptor&& other) = default;
    };
}
