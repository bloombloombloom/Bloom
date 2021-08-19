#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class TargetExecutionResumed: public Event
    {
    public:
        static inline EventType type = EventType::TARGET_EXECUTION_RESUMED;
        static inline const std::string name = "TargetExecutionResumed";

        TargetExecutionResumed() = default;

        [[nodiscard]] EventType getType() const override {
            return TargetExecutionResumed::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetExecutionResumed::name;
        }
    };
}
