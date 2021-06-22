#pragma once

#include <cstdint>
#include <string>

#include "Event.hpp"
#include "src/Targets/TargetBreakpoint.hpp"

namespace Bloom::Events
{
    class RemoveBreakpointOnTarget: public Event
    {
    public:
        static inline const std::string name = "RemoveBreakpointOnTarget";
        Targets::TargetBreakpoint breakpoint;

        [[nodiscard]] std::string getName() const override {
            return RemoveBreakpointOnTarget::name;
        }
    };
}
