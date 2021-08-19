#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class DebugSessionFinished: public Event
    {
    public:
        static inline EventType type = EventType::DEBUG_SESSION_FINISHED;
        static inline const std::string name = "DebugSessionFinished";

        [[nodiscard]] EventType getType() const override {
            return DebugSessionFinished::type;
        }

        [[nodiscard]] std::string getName() const override {
            return DebugSessionFinished::name;
        }
    };
}
