#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class StopTargetExecution: public Event
    {
    public:
        static inline EventType type = EventType::STOP_TARGET_EXECUTION;
        static inline const std::string name = "StopTargetExecution";

        [[nodiscard]] EventType getType() const override {
            return StopTargetExecution::type;
        }

        [[nodiscard]] std::string getName() const override {
            return StopTargetExecution::name;
        }
    };
}
