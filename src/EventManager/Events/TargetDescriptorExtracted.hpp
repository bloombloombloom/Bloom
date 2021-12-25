#pragma once

#include <string>

#include "Event.hpp"
#include "src/Targets/TargetDescriptor.hpp"

namespace Bloom::Events
{
    class TargetDescriptorExtracted: public Event
    {
    public:
        static constexpr EventType type = EventType::TARGET_DESCRIPTOR_EXTRACTED;
        static inline const std::string name = "TargetDescriptorExtracted";
        Targets::TargetDescriptor targetDescriptor;

        [[nodiscard]] EventType getType() const override {
            return TargetDescriptorExtracted::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetDescriptorExtracted::name;
        }
    };
}
