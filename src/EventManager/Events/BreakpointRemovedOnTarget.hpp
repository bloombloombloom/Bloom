#pragma once

#include <string>

#include "Event.hpp"

namespace Bloom::Events
{
    class BreakpointRemovedOnTarget: public Event
    {
    public:
        static inline EventType type = EventType::BREAKPOINT_REMOVED_ON_TARGET;
        static inline const std::string name = "BreakpointRemovedOnTarget";

        [[nodiscard]] EventType getType() const override {
            return BreakpointRemovedOnTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return BreakpointRemovedOnTarget::name;
        }
    };
}
