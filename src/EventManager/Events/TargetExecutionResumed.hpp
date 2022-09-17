#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class TargetExecutionResumed: public Event
    {
    public:
        static constexpr EventType type = EventType::TARGET_EXECUTION_RESUMED;
        static inline const std::string name = "TargetExecutionResumed";

        bool stepping = false;

        explicit TargetExecutionResumed(bool stepping)
            : stepping(stepping)
        {};

        [[nodiscard]] EventType getType() const override {
            return TargetExecutionResumed::type;
        }

        [[nodiscard]] std::string getName() const override {
            return TargetExecutionResumed::name;
        }
    };
}
