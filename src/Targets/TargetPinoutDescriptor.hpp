#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "TargetPinDescriptor.hpp"

namespace Targets
{
    enum class TargetPinoutType: std::uint8_t
    {
        SOIC,
        SSOP,
        DIP,
        QFN,
        QFP,
        DUAL_ROW_QFN,
        MLF,
        BGA,
    };

    struct TargetPinoutDescriptor
    {
    public:
        std::string key;
        std::string name;
        TargetPinoutType type;
        std::vector<TargetPinDescriptor> pinDescriptors;

        TargetPinoutDescriptor(
            const std::string& key,
            const std::string& name,
            TargetPinoutType type,
            std::vector<TargetPinDescriptor>&& pinDescriptors
        );

        TargetPinoutDescriptor(const TargetPinoutDescriptor& other) = delete;
        TargetPinoutDescriptor& operator = (const TargetPinoutDescriptor& other) = delete;

        TargetPinoutDescriptor(TargetPinoutDescriptor&& other) noexcept = default;
        TargetPinoutDescriptor& operator = (TargetPinoutDescriptor&& other) = default;

        bool operator == (const TargetPinoutDescriptor& other) const;
    };
}
