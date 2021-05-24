#pragma once

#include "Event.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Events
{
    class MemoryWrittenToTarget: public Event
    {
    public:
        static inline const std::string name = "MemoryWrittenToTarget";

        std::string getName() const override {
            return MemoryWrittenToTarget::name;
        }

        MemoryWrittenToTarget() = default;
    };
}
