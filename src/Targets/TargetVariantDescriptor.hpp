#pragma once

#include <string>

namespace Targets
{
    struct TargetVariantDescriptor
    {
    public:
        std::string name;
        std::string pinoutKey;

        TargetVariantDescriptor(
            const std::string& name,
            const std::string& pinoutKey
        );

        TargetVariantDescriptor(const TargetVariantDescriptor& other) = delete;
        TargetVariantDescriptor& operator = (const TargetVariantDescriptor& other) = delete;

        TargetVariantDescriptor(TargetVariantDescriptor&& other) noexcept = default;
        TargetVariantDescriptor& operator = (TargetVariantDescriptor&& other) noexcept = default;
    };
}
