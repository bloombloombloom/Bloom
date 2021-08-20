#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"
#include "BreakpointRemovedOnTarget.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

namespace Bloom::Events
{
    class RemoveBreakpointOnTarget: public Event
    {
    public:
        using TargetControllerResponseType = BreakpointRemovedOnTarget;

        static inline EventType type = EventType::REMOVE_BREAKPOINT_ON_TARGET;
        static inline const std::string name = "RemoveBreakpointOnTarget";
        Targets::TargetBreakpoint breakpoint;

        [[nodiscard]] EventType getType() const override {
            return RemoveBreakpointOnTarget::type;
        }

        [[nodiscard]] std::string getName() const override {
            return RemoveBreakpointOnTarget::name;
        }
    };
}
