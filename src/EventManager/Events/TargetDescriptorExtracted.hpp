#pragma once

#include <cstdint>

#include "Event.hpp"
#include "src/Targets/TargetDescriptor.hpp"

namespace Bloom::Events
{
    class TargetDescriptorExtracted: public Event
    {
    public:
        static inline const std::string name = "TargetDescriptorExtracted";
        Targets::TargetDescriptor targetDescriptor;

        std::string getName() const override {
            return TargetDescriptorExtracted::name;
        }
    };
}
