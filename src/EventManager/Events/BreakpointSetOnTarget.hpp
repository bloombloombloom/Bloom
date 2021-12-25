#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class BreakpointSetOnTarget: public Event
    {
    public:
        static constexpr EventType type = EventType::BREAKPOINT_SET_ON_TARGET;
        static inline const std::string name = "BreakpointSetOnTarget";

        [[nodiscard]] EventType getType() const override {
            return BreakpointSetOnTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return BreakpointSetOnTarget::name;
        }
    };
}
