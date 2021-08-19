#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

namespace Bloom::Events
{
    class SetBreakpointOnTarget: public Event
    {
    public:
        static inline EventType type = EventType::SET_BREAKPOINT_ON_TARGET;
        static inline const std::string name = "SetBreakpointOnTarget";
        Targets::TargetBreakpoint breakpoint;

        [[nodiscard]] EventType getType() const override {
            return SetBreakpointOnTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return SetBreakpointOnTarget::name;
        }
    };
}
