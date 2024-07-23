#pragma once

#include <cstdint>
#include <string>

namespace Targets
{
    enum class TargetPinType: std::uint8_t
    {
        GPIO,
        GND,
        VCC,
        OTHER,
    };

    struct TargetPinDescriptor
    {
    public:
        std::string padName;
        std::string position;
        TargetPinType type = TargetPinType::OTHER;

        TargetPinDescriptor(
            const std::string& padName,
            const std::string& position,
            TargetPinType type
        );

        TargetPinDescriptor(const TargetPinDescriptor& other) = delete;
        TargetPinDescriptor& operator = (const TargetPinDescriptor& other) = delete;

        TargetPinDescriptor(TargetPinDescriptor&& other) noexcept = default;
        TargetPinDescriptor& operator = (TargetPinDescriptor&& other) = default;
    };
}
