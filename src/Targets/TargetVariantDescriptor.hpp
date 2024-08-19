#pragma once

#include <string>

namespace Targets
{
    struct TargetVariantDescriptor
    {
    public:
        const std::string key;
        std::string name;
        std::string pinoutKey;

        TargetVariantDescriptor(
            const std::string& key,
            const std::string& name,
            const std::string& pinoutKey
        );

        TargetVariantDescriptor(const TargetVariantDescriptor& other) = delete;
        TargetVariantDescriptor& operator = (const TargetVariantDescriptor& other) = delete;

        TargetVariantDescriptor(TargetVariantDescriptor&& other) noexcept = default;
    };
}
