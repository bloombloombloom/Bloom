#pragma once

#include <string>

#include "Event.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Events
{
    class MemoryWrittenToTarget: public Event
    {
    public:
        static inline EventType type = EventType::MEMORY_WRITTEN_TO_TARGET;
        static inline const std::string name = "MemoryWrittenToTarget";

        MemoryWrittenToTarget() = default;

        [[nodiscard]] EventType getType() const override {
            return MemoryWrittenToTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return MemoryWrittenToTarget::name;
        }
    };
}
