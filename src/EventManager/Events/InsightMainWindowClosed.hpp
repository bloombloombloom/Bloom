#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class InsightMainWindowClosed: public Event
    {
    public:
        static constexpr EventType type = EventType::INSIGHT_MAIN_WINDOW_CLOSED;
        static const inline std::string name = "InsightMainWindowClosed";

        [[nodiscard]] EventType getType() const override {
            return InsightMainWindowClosed::type;
        }

        [[nodiscard]] std::string getName() const override {
            return InsightMainWindowClosed::name;
        }
    };
}
