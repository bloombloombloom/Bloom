#pragma once

#include <string>

#include "Event.hpp"
#include "src/Targets/TargetMemory.hpp"

namespace Bloom::Events
{
    class MemoryRetrievedFromTarget: public Event
    {
    public:
        static inline EventType type = EventType::MEMORY_RETRIEVED_FROM_TARGET;
        static inline const std::string name = "MemoryRetrievedFromTarget";
        Targets::TargetMemoryBuffer data;

        [[nodiscard]] EventType getType() const override {
            return MemoryRetrievedFromTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return MemoryRetrievedFromTarget::name;
        }
    };
}
