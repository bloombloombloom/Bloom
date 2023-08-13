#pragma once

#include <string>

#include "Event.hpp"

namespace Events
{
    class DebugSessionFinished: public Event
    {
    public:
        static constexpr EventType type = EventType::DEBUG_SESSION_FINISHED;
        static const inline std::string name = "DebugSessionFinished";

        [[nodiscard]] EventType getType() const override {
            return DebugSessionFinished::type;
        }

        [[nodiscard]] std::string getName() const override {
            return DebugSessionFinished::name;
        }
    };
}
