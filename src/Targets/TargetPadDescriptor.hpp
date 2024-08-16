#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Targets
{
    enum class TargetPadType: std::uint8_t
    {
        GPIO,
        GND,
        VCC,
        OTHER,
    };

    using TargetPadId = std::size_t;

    struct TargetPadDescriptor
    {
    public:
        const TargetPadId id;
        const std::string key;
        std::string name;
        TargetPadType type = TargetPadType::OTHER;

        TargetPadDescriptor(
            const std::string& key,
            const std::string& name,
            TargetPadType type
        );

        TargetPadDescriptor(const TargetPadDescriptor& other) = delete;
        TargetPadDescriptor& operator = (const TargetPadDescriptor& other) = delete;

        TargetPadDescriptor(TargetPadDescriptor&& other) noexcept = default;

        bool operator == (const TargetPadDescriptor& other) const;
        bool operator != (const TargetPadDescriptor& other) const;

        static TargetPadId generateId(const std::string& padKey);
    };

    using TargetPadDescriptors = std::vector<TargetPadDescriptor*>;
}
