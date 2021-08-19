#pragma once

#include <string>

#include "Event.hpp"

#include "TargetDescriptorExtracted.hpp"

namespace Bloom::Events
{
    class ExtractTargetDescriptor: public Event
    {
    public:
        using TargetControllerResponseType = TargetDescriptorExtracted;

        static inline EventType type = EventType::EXTRACT_TARGET_DESCRIPTOR;
        static inline const std::string name = "ExtractTargetDescriptor";

        [[nodiscard]] EventType getType() const override {
            return ExtractTargetDescriptor::type;
        }

        [[nodiscard]] std::string getName() const override {
            return ExtractTargetDescriptor::name;
        }
    };
}
