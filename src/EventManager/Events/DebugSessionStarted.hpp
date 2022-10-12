#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class DebugSessionStarted: public Event
    {
    public:
        static constexpr EventType type = EventType::DEBUG_SESSION_STARTED;
        static const inline std::string name = "DebugSessionStarted";

        [[nodiscard]] EventType getType() const override {
            return DebugSessionStarted::type;
        }

        [[nodiscard]] std::string getName() const override {
            return DebugSessionStarted::name;
        }
    };
}
